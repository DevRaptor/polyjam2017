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

	dynamic_world->setGravity(btVector3(0, -10, 0));

	GameModule::audio->AddSound("music1", "data/sounds/music1.wav");
	GameModule::audio->AddSound("boom2", "data/sounds/boom2.wav");
	GameModule::audio->AddSound("boom3", "data/sounds/boom3.wav");
	GameModule::audio->AddSound("boom1", "data/sounds/boom1.wav");
	GameModule::audio->AddSound("wood1", "data/sounds/destrWood.wav");

	GameModule::audio->AddSound("lady0", "data/sounds/quotes/lady0.wav");
	GameModule::audio->AddSound("maskman0", "data/sounds/quotes/maskman0.wav");
	GameModule::audio->AddSound("oldboy0", "data/sounds/quotes/oldboy0.wav");
	GameModule::audio->AddSound("pirate0", "data/sounds/quotes/pirate0.wav");

	GameModule::audio->SetVolumeMusic(100);
	GameModule::audio->PlaySound("music1");
	GameModule::audio->SetVolumeChunk("wood1", 15);

	AddFloor();

	MainMenuGui();

	InitGameplay();
}

GameState::~GameState()
{
	Logger::Log("Close gameplay with: 0 points\n");
}

void GameState::Update(std::chrono::milliseconds delta_time)
{
	float delta = delta_time.count() / 1000.0f; //in seconds
	dynamic_world->stepSimulation(delta, 10);

	//TURNSYSTEM - stateslike
	
	if (players[activeplayerid]->AlreadyShot())
	{
		if (!blockshooting)
		{
			blockshooting = true;
			ResetDestructTimer();
		}
		//camera change to show destructions
	}

	if ((blockshooting && 
		(DestructionsEnded() || GameModule::input->GetKeyState(SDL_SCANCODE_RSHIFT))) ||
		(std::chrono::high_resolution_clock::now() > playertimer))
	{	
		FadeInEffect();
	}

	if (fade && GameModule::input->GetKeyState(SDL_SCANCODE_SPACE))
	{
		fade = false;
		NextPlayer();
	}

	if (fade)
	{
		if(fadeout_coef > 0.0f)
			fadeout_coef -= fadeout_speed * delta;
	}
	else if (fadeout_coef < 1.0f)
	{
		fadeout_coef += fadeout_speed * delta;
		ShowNextPlayer(false, activeplayerid);//(activeplayerid+1)%players.size());
	}

	if (fadeout_coef <= 0.0f)
	{
		ShowNextPlayer(true, activeplayerid);//(activeplayerid+1)%players.size());
	}

	for (std::size_t i = 0; i < players.size(); ++i)
	{
		if (i == activeplayerid)
		{
			if (!players[i]->GetIsEnabled())
			{
				players[i]->SetIsEnabled(true);
				NextPlayer();
			}

			if (players[i]->GetHasWon())
			{
				//for (auto& obstacle : entities)
				//{
					//obstacle->Destroy();
				//}
			}

			if (!blockinput)
			{
				if (GameModule::input->IsLeftMouseButtonPressed() && !blockshooting)
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

	std::vector<std::pair<btVector3, double >> explosion_positions;

	auto it = entities.begin();
	while (it != entities.end())
	{
		if ((*it)->IsDestroyed())
		{
			if ((*it)->GetType() == EntityType::OBSTACLE_EXPLOSIVE)
			{
				
				if (std::chrono::high_resolution_clock::now() > (*it)->GetTimeOfDeath() + std::chrono::milliseconds(GameModule::resources->GetIntParameter("explosion_delay")))
				{
					explosion_positions.push_back({ (*it)->GetPhysicPosition(), (*it)->GetWaveRadius() });
					players[activeplayerid]->points += (*it)->points;
					it = entities.erase(it);
				}
			}
			else
			{
				if((*it)->GetType() != EntityType::PARTICLE)
					GameModule::audio->PlaySound("wood1");
				players[activeplayerid]->points += (*it)->points;
				it = entities.erase(it);
			}
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
		Explosion(pos.first, pos.second);
	}

	static std::chrono::high_resolution_clock::time_point restart_timer = std::chrono::high_resolution_clock::now();
	if (GameModule::input->GetKeyState(SDL_SCANCODE_R)
		&& (std::chrono::high_resolution_clock::now() > restart_timer))
	{
		RestartGameplay();

		restart_timer = std::chrono::high_resolution_clock::now() + std::chrono::seconds(1);
	}

	static std::chrono::high_resolution_clock::time_point points_timer = std::chrono::high_resolution_clock::now();
	if (GameModule::input->GetKeyState(SDL_SCANCODE_P)
		&& (std::chrono::high_resolution_clock::now() > points_timer))
	{
		std::cout << "=====Scoreboard======\n";
		for(int i = 0; i < players.size(); i++)
		{
			std::cout << "player " << i << " has: " << players[i]->points << "\n";
		}

		points_timer = std::chrono::high_resolution_clock::now() + std::chrono::seconds(1);
	}

	if (players.size() > 0)
	{
		camera.Translate(players[activeplayerid]->GetPosition() + glm::vec3(0, 10, 0));
		camera.Shake();
		//camera.LookAt(players.front()->GetPosition());
	}

	CheckTriggers();
}

void GameState::FadeInEffect()
{
	blockinput = true;
	fade = true;
}


void GameState::NextPlayer()
{

	if(activeplayerid != -1) //first iteration AccessViolation exception
		players[activeplayerid]->QuitShooting();

	++activeplayerid;
	activeplayerid %= players.size();

	std::string temp = "";
	if (players[activeplayerid]->currentcharacter == 0)
		temp += "oldboy";
	else if (players[activeplayerid]->currentcharacter == 1)
		temp += "lady";
	else if (players[activeplayerid]->currentcharacter == 2)
		temp += "maskman";
	else
		temp += "pirate";

	temp += std::to_string((rand() % GameModule::resources->GetIntParameter("quotes")));

	std::cout << temp << "\n";

	GameModule::audio->PlaySound(temp);

	//fade out - probably another timer

	ResetTurnTimer();

	blockshooting = false;
	blockinput = false;
}

void GameState::ResetTurnTimer()
{
	playertimer = std::chrono::high_resolution_clock::now() +
		std::chrono::seconds(GameModule::resources->GetIntParameter("turnlenght"));
}

void GameState::ResetDestructTimer()
{
	destruct_timer = std::chrono::high_resolution_clock::now() +
		std::chrono::seconds(GameModule::resources->GetIntParameter("destructime"));
}

bool GameState::DestructionsEnded()
{	
	if (std::chrono::high_resolution_clock::now() > destruct_timer)
	{
		ResetDestructTimer();

		return true;
	}
	return false;
}

void GameState::SpawnObstaclesRand()
{
	/*
	x goes from 0 and below
	don't touch y
	z goes wherever you want
	0,0,0 is player spawn - don't spawn here
	*/
	int obstacles_amount = GameModule::resources->GetIntParameter("obstacles_amount");

	// How far from each side from any player obstacles can't spawn
	float player_safe_space_size = 5;


	float object_size = 3;

	int light_spawn_chance = GameModule::resources->GetIntParameter("light_spawn_chance");
	int heavy_spawn_chance = GameModule::resources->GetIntParameter("heavy_spawn_chance");
	int expl_spawn_chance = GameModule::resources->GetIntParameter("expl_spawn_chance");

	std::uniform_int_distribution<> random_spawner(0, (light_spawn_chance + heavy_spawn_chance + expl_spawn_chance));
	std::uniform_real_distribution<> random_position(-100, 100);

	glm::vec3 scale(1, 1, 1);


	static const double explosionRadius = GameModule::resources->GetIntParameter("explosion_radius");//3;

	for (int i = 0; i <= obstacles_amount; i++)
	{
		bool is_position_near_player = false;

		float current_x = random_position(GameModule::random_gen);
		float current_z = random_position(GameModule::random_gen);

		for (std::size_t i = 0; i < players.size(); ++i)
		{
			glm::vec3 player_pos = players[i]->GetPosition();
			if (!(((current_x + player_safe_space_size) < player_pos.x || (current_x - player_safe_space_size) > player_pos.x)
				|| ((current_z + player_safe_space_size) < player_pos.z || (current_z - player_safe_space_size) > player_pos.z)))
			{
				is_position_near_player = true;
				break;
			}

		}
		if (!is_position_near_player)
		{
			glm::vec3 pos(current_x, 0, current_z);

			int rand_obstacle_type = random_spawner(GameModule::random_gen);

			if (rand_obstacle_type < expl_spawn_chance)
			{
				auto obj = std::make_shared<Obstacle>(EntityType::OBSTACLE_EXPLOSIVE, dynamic_world, pos, scale, explosionRadius);
				obj->Init();
				entities.push_back(obj);
			}
			else if (rand_obstacle_type < expl_spawn_chance + heavy_spawn_chance)
			{
				auto obj = std::make_shared<Obstacle>(EntityType::OBSTACLE_HEAVY, dynamic_world, pos, scale, explosionRadius);
				obj->Init();
				entities.push_back(obj);
			}
			else
			{
				auto obj = std::make_shared<Obstacle>(EntityType::OBSTACLE_LIGHT, dynamic_world, pos, scale, explosionRadius);
				obj->Init();
				entities.push_back(obj);
			}
		}
	}
}

void GameState::SpawnObstaclesGrid()
{
	/*
	x goes from 0 and below
	don't touch y
	z goes wherever you want
	0,0,0 is player spawn - don't spawn here
	*/
	int obstacles_amount_per_wall = GameModule::resources->GetIntParameter("obstacles_amount");// 30;

	// How far from each side from any player obstacles can't spawn
	float player_safe_space_size = 5;


	float object_size = 3;

	int no_spawn_chance = GameModule::resources->GetIntParameter("no_spawn_chance");
	int light_spawn_chance = GameModule::resources->GetIntParameter("light_spawn_chance");
	int heavy_spawn_chance = GameModule::resources->GetIntParameter("heavy_spawn_chance");
	int expl_spawn_chance = GameModule::resources->GetIntParameter("expl_spawn_chance");

	std::uniform_int_distribution<> random_spawner(0, (no_spawn_chance + light_spawn_chance + heavy_spawn_chance + expl_spawn_chance));
	std::uniform_real_distribution<> random_position(-100, 100);
	std::uniform_real_distribution<> random_win_spawn(0.0f, 0.6f);

	glm::vec3 scale(1, 0.5, 1);
	glm::vec3 biggerScale(1, 0.1, 1);

	bool win_spawned = false;
	int win_spawn_pos = std::floorf( random_win_spawn(GameModule::random_gen) * obstacles_amount_per_wall * obstacles_amount_per_wall *
		((1.0f * no_spawn_chance )/ (light_spawn_chance + heavy_spawn_chance + expl_spawn_chance + no_spawn_chance)));
	

	static const double explosionRadius = GameModule::resources->GetIntParameter("explosion_radius");//3;



	for (int i = 0; i <= obstacles_amount_per_wall; i++)
	{
		for (int j = 0; j <= obstacles_amount_per_wall; j++)
		{
			bool is_position_near_player = false;

			float current_x = -20 + i * object_size;
			float current_z = -20 + j * object_size;

			for (std::size_t i = 0; i < players.size(); ++i)
			{
				glm::vec3 player_pos = players[i]->GetPosition();
				if (!(((current_x + player_safe_space_size) < player_pos.x || (current_x - player_safe_space_size) > player_pos.x)
				|| ((current_z + player_safe_space_size) < player_pos.z || (current_z - player_safe_space_size) > player_pos.z)))
				{
					is_position_near_player = true;
					break;
				}

			}
			if (!is_position_near_player)
			{
				glm::vec3 pos(current_x, 0, current_z);

				int rand_obstacle_type = random_spawner(GameModule::random_gen);

				if (rand_obstacle_type < expl_spawn_chance)
				{
					auto obj = std::make_shared<Obstacle>(EntityType::OBSTACLE_EXPLOSIVE, dynamic_world, pos, scale, explosionRadius);
					obj->Init();
					entities.push_back(obj);
				}
				else if (rand_obstacle_type < expl_spawn_chance + heavy_spawn_chance)
				{
					auto obj = std::make_shared<Obstacle>(EntityType::OBSTACLE_HEAVY, dynamic_world, pos, scale, explosionRadius);
					obj->Init();
					entities.push_back(obj);
				}
				else if (rand_obstacle_type < expl_spawn_chance + heavy_spawn_chance + light_spawn_chance)
				{
					auto obj = std::make_shared<Obstacle>(EntityType::OBSTACLE_LIGHT, dynamic_world, pos, scale, explosionRadius);
					obj->Init();
					entities.push_back(obj);
				}
				else
				{
					if (win_spawn_pos > 0)
						--win_spawn_pos;
					else if(!win_spawned)
					{
						auto obj = std::make_shared<Obstacle>(EntityType::OBSTACLE_WIN_CONDITION, dynamic_world, pos, scale, explosionRadius);
						obj->Init();
						entities.push_back(obj);
						std::cout << "Win spawned at " << pos.x << " " << pos.z << "\n";
						win_spawned = true;
					}
				}
				
			}
		}
	}

	if (!win_spawned)
	{
		std::cout << "Raptot - zjebales \n";
	}
}

void GameState::InitGameplay()
{
	obstacle_data.delay = std::chrono::milliseconds(obstacle_data.default_delay);

	ResetDestructTimer();
	ResetTurnTimer();

	for (int i = 0; i < GameModule::resources->GetIntParameter("playersamount"); i++)
	{
		AddPlayer(glm::vec3(i * 15, 0, i * 15));
	}
	activeplayerid = -1;
	NextPlayer(); //hack to init turntimer properly

	//SpawnObstaclesRand();
	SpawnObstaclesGrid();
}



GLfloat GameState::get_fadeout()
{
	return fadeout_coef;
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
	Logger::Log("Restart gameplay, points: 0\n");

	entities.clear();

	InitGameplay();
}

void GameState::Explosion(btVector3& pos, double radius)
{
	std::uniform_int_distribution<> random_sound(1, 3);
	std::string sound_name = "boom" + std::to_string(random_sound(GameModule::random_gen));

	GameModule::audio->PlaySound(sound_name);
	for (int i = 0; i < GameModule::resources->GetIntParameter("particles_quantity"); i++)
	{
		std::uniform_real_distribution<> random(-5.0, 5.0);
		std::uniform_real_distribution<> randpos(-0.5, 0.5);

		glm::vec3 pos(random(GameModule::random_gen) + pos.getX(), random(GameModule::random_gen) + pos.getY(),
			random(GameModule::random_gen) + pos.getZ());

		auto obj = std::make_shared<Obstacle>(EntityType::PARTICLE, dynamic_world,
			pos, glm::vec3(1, 1, 1), 0);
		obj->Init();
		entities.push_back(obj);
		obj->GetRigidBody()->applyCentralImpulse(btVector3(random(GameModule::random_gen), 10, 
			random(GameModule::random_gen)));
	}

	auto obj = std::make_shared<Obstacle>(EntityType::EXPLOSION, dynamic_world, glm::vec3(pos.getX(), pos.getY(), pos.getZ()), glm::vec3(radius, 0, radius), 0);
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

				if (obj0->GetType() == EntityType::OBSTACLE_WIN_CONDITION || obj1->GetType() == EntityType::OBSTACLE_WIN_CONDITION)
					continue;

				if (obj0->GetType() == EntityType::EXPLOSION 
					&& (obj1->GetType() == EntityType::OBSTACLE_HEAVY || obj1->GetType() == EntityType::OBSTACLE_EXPLOSIVE || obj1->GetType() == EntityType::OBSTACLE_LIGHT))
				{
					obj0->GetOwner()->Destroy();
					obj1->GetOwner()->Destroy();
					camera.StartShaking();
				}

				if (obj1->GetType() == EntityType::EXPLOSION
					&& (obj0->GetType() == EntityType::OBSTACLE_HEAVY || obj0->GetType() == EntityType::OBSTACLE_EXPLOSIVE || obj0->GetType() == EntityType::OBSTACLE_LIGHT))
				{
					obj0->GetOwner()->Destroy();
					obj1->GetOwner()->Destroy();
					camera.StartShaking();
				}
			}
		}
	}
}

void GameState::MainMenuGui()
{
	glm::vec2 pos(0, 0);
	glm::vec2 size(0.5, 0.5);

	players_graphics.push_back(std::make_shared<Mesh>("quad", "player_big1", pos, size));
	players_graphics.push_back(std::make_shared<Mesh>("quad", "player_big2", pos, size));
	players_graphics.push_back(std::make_shared<Mesh>("quad", "player_big3", pos, size));
	players_graphics.push_back(std::make_shared<Mesh>("quad", "player_big3", pos, size));
}

void GameState::ShowNextPlayer(bool show, int player_id)
{
	if (show)
	{
		//must add player_id

		next_player = players_graphics[player_id];
	}
	else
	{
		next_player = nullptr;
	}
}