#include "GameState.h"

#include <iostream>

#include "entity/Obstacle.h"
#include "entity/Ship.h"
#include "utility/Log.h"

#include "modules/GameModule.h"

GameState::GameState()
	: camera{90, 0.1, 100}
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

	//debug
	
	if (GameModule::input->GetKeyState(SDL_SCANCODE_SLASH))
	{
		++activeplayerid;
		activeplayerid %= 4;
		std::cout << " KURWA! " << activeplayerid << "\n";
	}

	auto it = players.begin();
	while (it != players.end())
	{
		if ((*it)->index == activeplayerid)
		{
			if (GameModule::input->GetKeyState(SDL_SCANCODE_SPACE))
			{
				(*it)->DoShoot();
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

			(*it)->Move(tempvec);
		}


		if ((*it)->IsDestroyed())
		{
			it = players.erase(it);
		}
		else
		{
			(*it)->Update();

			dynamic_world->contactTest((*it)->GetRigidBody(), callback);

			++it;
		}
	}


	it = entities.begin();
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


	if (std::chrono::high_resolution_clock::now() > obstacle_data.timer)
	{
		SpawnObstacle();
		obstacle_data.timer = std::chrono::high_resolution_clock::now() + obstacle_data.delay;
	}

	//change Obstacle delay time when player destroyed Obstacles
	int delay = obstacle_data.default_delay - (100 * Ship::points);
	obstacle_data.delay = std::chrono::milliseconds(delay > obstacle_data.min_delay ? delay : obstacle_data.min_delay);


	static std::chrono::high_resolution_clock::time_point restart_timer = std::chrono::high_resolution_clock::now();
	if (GameModule::input->GetKeyState(SDL_SCANCODE_R)
		&& (std::chrono::high_resolution_clock::now() > restart_timer))
	{
		RestartGameplay();

		restart_timer = std::chrono::high_resolution_clock::now() + std::chrono::seconds(1);
	}

	if (players.size() > 0)
	{
		camera.Translate(players[activeplayerid]->GetPosition() + glm::vec3(0, 10, 0));
		//camera.LookAt(players.front()->GetPosition());
	}
}

void GameState::SpawnObstacle()
{
	std::uniform_real_distribution<> random_distortion(1.0f, obstacle_data.distortion);
	std::uniform_real_distribution<> random_scale(obstacle_data.scale_min, obstacle_data.scale_max);
	float size = random_scale(GameModule::random_gen);

	glm::vec3 scale(size, size, size * random_distortion(GameModule::random_gen));

	std::uniform_real_distribution<> rand_pos_z(-obstacle_data.pos_z, obstacle_data.pos_z);

	//must lower Obstacle position by half of size
	glm::vec3 pos(obstacle_data.pos_x, -size / 2.0f, rand_pos_z(GameModule::random_gen));

	auto obj = std::make_shared<Obstacle>(dynamic_world, pos, scale);
	obj->Init();
	entities.push_back(obj);
}

void GameState::InitGameplay()
{
	obstacle_data.delay = std::chrono::milliseconds(obstacle_data.default_delay);

	for (int i = 0; i < 4; i++) //raptor said 4
	{
		AddPlayer(glm::vec3(i*15,0,i*15));
	}
	

	activeplayerid = 0;
}

void GameState::AddPlayer(glm::vec3 startpos)
{
	auto obj = std::make_shared<Ship>(dynamic_world, startpos, entities);
	obj->Init();
	players.push_back(obj);
	camera.Translate(players.front()->GetPosition() + glm::vec3(0, 10, 0));
	camera.LookAt(players.front()->GetPosition());
}

void GameState::RestartGameplay()
{
	Logger::Log("Restart gameplay, points: ", Ship::points, "\n");

	players.clear();
	entities.clear();

	InitGameplay();
}
