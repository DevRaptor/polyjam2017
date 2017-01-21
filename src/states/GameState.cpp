#include "GameState.h"

#include <iostream>

#include "entity/Meteor.h"
#include "entity/Ship.h"
#include "utility/Log.h"

#include "modules/GameModule.h"

GameState::GameState()
	: camera{90, 0.1, 100}
{
	meteor_data.pos_x = -GameModule::resources->GetFloatParameter("camera_pos_y") * 1.2f;
	meteor_data.pos_z = GameModule::resources->GetFloatParameter("camera_pos_y") / 2.0f;

	meteor_data.scale_min = GameModule::resources->GetFloatParameter("meteor_scale_min");
	meteor_data.scale_max = GameModule::resources->GetFloatParameter("meteor_scale_max");
	meteor_data.distortion = GameModule::resources->GetFloatParameter("meteor_max_distortion");

	meteor_data.default_delay = GameModule::resources->GetIntParameter("meteor_delay");
	meteor_data.min_delay = GameModule::resources->GetIntParameter("meteor_delay_min");

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

	auto it = players.begin();
	while (it != players.end())
	{
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


	if (std::chrono::high_resolution_clock::now() > meteor_data.timer)
	{
		SpawnMeteor();
		meteor_data.timer = std::chrono::high_resolution_clock::now() + meteor_data.delay;
	}

	//change meteor delay time when player destroyed meteors
	int delay = meteor_data.default_delay - (100 * Ship::points);
	meteor_data.delay = std::chrono::milliseconds(delay > meteor_data.min_delay ? delay : meteor_data.min_delay);


	static std::chrono::high_resolution_clock::time_point restart_timer = std::chrono::high_resolution_clock::now();
	if (GameModule::input->GetKeyState(SDL_SCANCODE_R)
		&& (std::chrono::high_resolution_clock::now() > restart_timer))
	{
		RestartGameplay();

		restart_timer = std::chrono::high_resolution_clock::now() + std::chrono::seconds(1);
	}

	/*if (players.size() > 0)
	{
		camera.Translate(players.front()->GetPosition() + glm::vec3(0, 10, 0));
		camera.LookAt(players.front()->GetPosition());
	}*/
}

void GameState::SpawnMeteor()
{
	std::uniform_real_distribution<> random_distortion(1.0f, meteor_data.distortion);
	std::uniform_real_distribution<> random_scale(meteor_data.scale_min, meteor_data.scale_max);
	float size = random_scale(GameModule::random_gen);

	glm::vec3 scale(size, size, size * random_distortion(GameModule::random_gen));

	std::uniform_real_distribution<> rand_pos_z(-meteor_data.pos_z, meteor_data.pos_z);

	//must lower meteor position by half of size
	glm::vec3 pos(meteor_data.pos_x, -size / 2.0f, rand_pos_z(GameModule::random_gen));

	auto meteor = std::make_shared<Meteor>(dynamic_world, pos, scale);
	meteor->Init();
	entities.push_back(meteor);
}

void GameState::InitGameplay()
{
	meteor_data.delay = std::chrono::milliseconds(meteor_data.default_delay);

	auto obj = std::make_shared<Ship>(dynamic_world, glm::vec3(0, 0, 0), entities);
	obj->Init();
	players.push_back(obj);
	camera.Translate(players.front()->GetPosition() + glm::vec3(0, 10, 0));
	camera.LookAt(players.front()->GetPosition());
}

void GameState::RestartGameplay()
{
	Logger::Log("Restart gameplay, points: ", Ship::points, "\n");

	entities.clear();

	InitGameplay();
}
