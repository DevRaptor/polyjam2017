#include "GameState.h"

#include <iostream>

#include "entity/Obstacle.h"
#include "entity/Ship.h"
#include "utility/Log.h"

#include "modules/GameModule.h"

GameState::GameState() : camera{90, 0.1, 100}
{
	obstacle_data.pos_x = -GameModule::resources->GetFloatParameter("camera_pos_y") * 1.2f;
	obstacle_data.pos_z = GameModule::resources->GetFloatParameter("camera_pos_y") / 2.0f;

	obstacle_data.scale_min = GameModule::resources->GetFloatParameter("meteor_scale_min");
	obstacle_data.scale_max = GameModule::resources->GetFloatParameter("meteor_scale_max");
	obstacle_data.distortion = GameModule::resources->GetFloatParameter("meteor_max_distortion");

	obstacle_data.default_delay = GameModule::resources->GetIntParameter("meteor_delay");
	obstacle_data.min_delay = GameModule::resources->GetIntParameter("meteor_delay_min");

	broad_phase = std::make_unique<btDbvtBroadphase>();
	collision_config = std::make_unique<btDefaultCollisionConfiguration>();
	dispatcher = std::make_unique<btCollisionDispatcher>(collision_config.get());
	solver = std::make_unique<btSequentialImpulseConstraintSolver>();

	dynamic_world = std::make_shared<btDiscreteDynamicsWorld>(dispatcher.get(), broad_phase.get(),
		solver.get(), collision_config.get());

	dynamic_world->setGravity(btVector3(0, 0, 0));

	InitGameplay();
}

GameState::~GameState()
{
	Logger::Log("Close gameplay with: ", Ship::points, " points\n");
}

void GameState::Update(std::chrono::milliseconds delta_time)
{
	float delta = delta_time.count() / 1000.0f; //in seconds
	dynamic_world->stepSimulation(delta, 10);

	for (std::size_t i = 0; i < players.size(); ++i)
	{
		if (i == activeplayerid)
		{
			if (GameModule::input->GetKeyState(SDL_SCANCODE_SPACE))
			{
				players[i]->DoShoot();
			}

			btVector3* tempvec = new btVector3(0, 0, 0);

			if (GameModule::input->GetKeyState(SDL_SCANCODE_W) ||
				GameModule::input->GetKeyState(SDL_SCANCODE_UP))
				tempvec->setX(-1);
			else if (GameModule::input->GetKeyState(SDL_SCANCODE_S) ||
				GameModule::input->GetKeyState(SDL_SCANCODE_DOWN))
				tempvec->setX(1);

			if (GameModule::input->GetKeyState(SDL_SCANCODE_A) ||
				GameModule::input->GetKeyState(SDL_SCANCODE_LEFT))
				tempvec->setZ(1);
			else if (GameModule::input->GetKeyState(SDL_SCANCODE_D) ||
				GameModule::input->GetKeyState(SDL_SCANCODE_RIGHT))
				tempvec->setZ(-1);

			players[i]->Move(tempvec);
		}


		if (players[i]->IsDestroyed())
		{
			players.erase(players.begin() + i);
			--i;
		}
		else
		{
			players[i]->Update();

			dynamic_world->contactTest(players[i]->GetRigidBody(), callback);
		}
	}


	auto it = entities.begin();
	while (it != entities.end())
	{
		if ((*it)->IsDestroyed())
		{
			it = entities.erase(it);
		}
		else
		{
			(*it)->Update();

			dynamic_world->contactTest((*it)->GetRigidBody(), callback);

			++it;
		}
	}


	static std::chrono::high_resolution_clock::time_point restart_timer = std::chrono::high_resolution_clock::now();
	if (GameModule::input->GetKeyState(SDL_SCANCODE_R)
		&& (std::chrono::high_resolution_clock::now() > restart_timer))
	{
		RestartGameplay();

		restart_timer = std::chrono::high_resolution_clock::now() + std::chrono::seconds(1);
	}

	if (players.size() > 0)
	{
		camera.Translate(players.front()->GetPosition() + glm::vec3(0, 10, 0));
	}
}

void GameState::SpawnObstacles()
{

	/*
	x goes from 0 and below
	don't touch y
	z goes wherever you want
	0,0,0 is player spawn - don't spawn here
	*/
	int obstacles_amount_per_wall = 15;

	float min_x = -35;
	float max_x = -8;
	float min_z = -35;
	float max_z = -8;

	float object_size = 3;

	std::uniform_real_distribution<> random_spawn_start_x(min_x, max_x);
	std::uniform_real_distribution<> random_spawn_start_z(min_z, max_z);

	float start_x = random_spawn_start_x(GameModule::random_gen);
	float start_z = random_spawn_start_z(GameModule::random_gen);

	glm::vec3 scale(1, 1, 1);

	for (int i = 0; i <= obstacles_amount_per_wall; i++)
	{
		for (int j = 0; j <= obstacles_amount_per_wall; j++)
		{
			if (i == 0 || i == obstacles_amount_per_wall
				|| j == 0 || j == obstacles_amount_per_wall)
			{
				glm::vec3 pos(start_x + i * object_size, 0, start_z + j * object_size);

				auto obj = std::make_shared<Obstacle>(dynamic_world, pos, scale);
				obj->Init();
				entities.push_back(obj);
			}
		}
	}
}

void GameState::InitGameplay()
{
	obstacle_data.delay = std::chrono::milliseconds(obstacle_data.default_delay);

	activeplayerid = 0;

	auto obj = std::make_shared<Ship>(dynamic_world, glm::vec3(0, 0, 0), entities);
	obj->Init();
	players.push_back(obj);
	camera.Translate(players.front()->GetPosition() + glm::vec3(0, 10, 0));
	camera.LookAt(players.front()->GetPosition());
	SpawnObstacles();
}

void GameState::RestartGameplay()
{
	Logger::Log("Restart gameplay, points: ", Ship::points, "\n");

	entities.clear();

	InitGameplay();
}
