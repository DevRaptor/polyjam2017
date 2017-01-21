#include "GameState.h"

#include <iostream>

#include "entity/Obstacle.h"
#include "entity/Ship.h"
#include "utility/Log.h"

#include "modules/GameModule.h"

GameState::GameState() : camera{ 90, 0.1, 100 }
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

	AddFloor();

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



	static std::chrono::high_resolution_clock::time_point switchtimer = std::chrono::high_resolution_clock::now();
	if (GameModule::input->GetKeyState(SDL_SCANCODE_SLASH)
		&& (std::chrono::high_resolution_clock::now() > switchtimer))
	{
		++activeplayerid;
		activeplayerid %= players.size();

		switchtimer = std::chrono::high_resolution_clock::now() + std::chrono::seconds(1);
	}

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

	std::vector<btVector3> explosion_positions;

	auto it = entities.begin();
	while (it != entities.end())
	{
		if ((*it)->IsDestroyed())
		{
			if ((*it)->GetType() == EntityType::OBSTACLE_HEAVY)
			{
				explosion_positions.push_back((*it)->GetPhysicPosition());
			}

			it = entities.erase(it);
		}
		else
		{
			(*it)->Update();

			dynamic_world->contactTest((*it)->GetRigidBody(), callback);

			++it;
		}
	}


	for (auto& pos : explosion_positions)
	{
		Explosion(pos);
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
		camera.Translate(players[activeplayerid]->GetPosition() + glm::vec3(0, 10, 0));
		//camera.LookAt(players.front()->GetPosition());
	}




	CheckTriggers();
	
}

void GameState::SpawnObstacles()
{

	/*
	x goes from 0 and below
	don't touch y
	z goes wherever you want
	0,0,0 is player spawn - don't spawn here
	*/
	int obstacles_amount_per_wall = 100;

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

				auto obj = std::make_shared<Obstacle>(EntityType::OBSTACLE_HEAVY, dynamic_world, pos, scale);
				obj->Init();
				entities.push_back(obj);
			}
		}
	}
}

void GameState::InitGameplay()
{
	obstacle_data.delay = std::chrono::milliseconds(obstacle_data.default_delay);

	for (int i = 0; i < 5; i++) //raptor said 4
	{
		AddPlayer(glm::vec3(i * 15, 0, i * 15));
	}
	activeplayerid = 0;

	SpawnObstacles();
}

void GameState::AddFloor()
{

	groundShape = std::make_shared<btStaticPlaneShape>(btVector3(0, 1, 0), 0);

	groundMotionState = std::make_shared<btDefaultMotionState>(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
	btRigidBody::btRigidBodyConstructionInfo
		groundRigidBodyCI(0, groundMotionState.get(), groundShape.get(), btVector3(0, 0, 0));
	groundRigidBody = std::make_shared<btRigidBody>(groundRigidBodyCI);
	dynamic_world->addRigidBody(groundRigidBody.get());

	btTransform transform;
	groundRigidBody->getMotionState()->getWorldTransform(transform);
	float matrix[16];
	transform.getOpenGLMatrix(matrix);

	floor_transform = glm::make_mat4(matrix);// *glm::scale(glm::mat4(1.0f), scale);

	floor_mesh = GameModule::resources->GetMesh("floor");

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

	entities.clear();

	InitGameplay();
}

void GameState::Explosion(btVector3& pos)
{

	auto obj = std::make_shared<Obstacle>(EntityType::PARTICLE, dynamic_world,
		glm::vec3(pos.getX(), pos.getY(), pos.getZ()), glm::vec3(1, 1, 1));
	obj->Init();
	entities.push_back(obj);

	obj = std::make_shared<Obstacle>(EntityType::EXPLOSION, dynamic_world, glm::vec3(pos.getX(), pos.getY(), pos.getZ()), glm::vec3(3, 3, 3));
	obj->Init();
	entities.push_back(obj);


}

void GameState::CheckTriggers()
{
	int numManifolds = dynamic_world->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; i++)
	{
		btPersistentManifold* contactManifold = dynamic_world->getDispatcher()->getManifoldByIndexInternal(i);
		auto  obA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
		auto  obB = static_cast<const btCollisionObject*>(contactManifold->getBody1());

		int numContacts = contactManifold->getNumContacts();
		for (int j = 0; j < numContacts; j++)
		{
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			if (pt.getDistance() < 0.f)
			{
				const btVector3& ptA = pt.getPositionWorldOnA();
				const btVector3& ptB = pt.getPositionWorldOnB();
				const btVector3& normalOnB = pt.m_normalWorldOnB;

				const RigidBody* obj0 = static_cast<const RigidBody*>(obA);
				const RigidBody* obj1 = static_cast<const RigidBody*>(obB);

				/*if (obj0->GetType() == EntityType::BULLET || obj1->GetType() == EntityType::BULLET)
				{
					obj0->GetOwner()->Destroy();
					obj1->GetOwner()->Destroy();
				}*/

				if (obj0->GetType() == EntityType::EXPLOSION && obj1->GetType() == EntityType::OBSTACLE_HEAVY)
				{
					obj0->GetOwner()->Destroy();
					obj1->GetOwner()->Destroy();
				}

				if (obj1->GetType() == EntityType::EXPLOSION && obj0->GetType() == EntityType::OBSTACLE_HEAVY)
				{
					obj0->GetOwner()->Destroy();
					obj1->GetOwner()->Destroy();
				}
			}
		}
	}
}