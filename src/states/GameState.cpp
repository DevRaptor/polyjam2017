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

	//music
	GameModule::audio->AddMusic("ambient", "data/sounds/music1.ogg");

	//effects
	GameModule::audio->AddSound("boom2", "data/sounds/boom2.wav");
	GameModule::audio->AddSound("boom3", "data/sounds/boom3.wav");
	GameModule::audio->AddSound("boom1", "data/sounds/boom1.wav");
	GameModule::audio->AddSound("wood1", "data/sounds/destrWood.wav");

	//quotes
	GameModule::audio->AddSound("lady0", "data/sounds/quotes/lady0.wav");
	GameModule::audio->AddSound("maskman0", "data/sounds/quotes/maskman0.wav");
	GameModule::audio->AddSound("oldboy0", "data/sounds/quotes/oldboy0.wav");
	GameModule::audio->AddSound("pirate0", "data/sounds/quotes/pirate0.wav");
	GameModule::audio->AddSound("lady1", "data/sounds/quotes/lady1.wav");
	GameModule::audio->AddSound("maskman1", "data/sounds/quotes/maskman1.wav");
	GameModule::audio->AddSound("oldboy1", "data/sounds/quotes/oldboy1.wav");
	GameModule::audio->AddSound("pirate1", "data/sounds/quotes/pirate1.wav");

	//volume
	GameModule::audio->SetVolumeChunk("wood1", 15);
	GameModule::audio->SetVolumeMusic(50);

	GameModule::audio->PlayMusic("ambient", 1000, 1, 1);

	AddFloor();

	MainMenuGui();
	start = true;
	InitGameplay();
}

GameState::~GameState()
{
	Logger::Log("Close gameplay with: 0 points\n");
}

void GameState::Update(std::chrono::milliseconds delta_time)
{
	GameModule::audio->Update();

	if (start)
	{
		if (GameModule::input->GetKeyState(SDL_SCANCODE_Z))
		{
			start = false;
			gui.clear();
		}
		else
			return;
	}

	for (int i = 0; i < players.size(); i++)
	{
		if (players[i]->points > win_points)
		{
			winner_id = i;
		}
	}

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
	}

	if ((blockshooting &&
		(DestructionsEnded() || GameModule::input->GetKeyState(SDL_SCANCODE_RSHIFT))) ||
		(std::chrono::high_resolution_clock::now() > playertimer))
	{
		FadeInEffect();
	}

	static std::chrono::high_resolution_clock::time_point space_timer = std::chrono::high_resolution_clock::now();
	if (fade && GameModule::input->GetKeyState(SDL_SCANCODE_SPACE) && (std::chrono::high_resolution_clock::now() > space_timer))
	{
		fade = false;
		NextPlayer();

		space_timer = std::chrono::high_resolution_clock::now() + std::chrono::seconds(1);
	}

	if (fade)
	{
		gameplay = false;
		if (fadeout_coef > 0.0f)
			fadeout_coef -= fadeout_speed * delta;

		if (winner_id != -1)
		{
			WinScreen();

			return;
		}
	}
	else if (fadeout_coef < 1.0f)
	{
		fadeout_coef += fadeout_speed * delta;
		int id = activeplayerid;
		/*std::cout << players[id]->GetIsEnabled() << "\n";
		if (!(players[id]->GetIsEnabled()))
		{
			std::cout << id << "\n";
			id = (id + 1) % players.size();
		}*/
		//std::cout << "id: " << id << "\n";
		ShowNextPlayer(false, id);//(activeplayerid+1)%players.size());
	}
	else
	{
		gameplay = true;
	}

	if (fadeout_coef <= 0.0f)
	{
		int id = activeplayerid;
		/*std::cout << players[id]->GetIsEnabled() << "ufo\n";
		while (!(players[id]->GetIsEnabled()))
		{
			std::cout << id << "\n";
			id = (id + 1) % players.size();
		}*/
		ShowNextPlayer(true, id);//(activeplayerid+1)%players.size());
		
	}

	for (std::size_t i = 0; i < players.size(); ++i)
	{
		if (i == activeplayerid)
		{
			/*if (!players[i]->GetIsEnabled())
			{
				players[i]->SetIsEnabled(true);
				NextPlayer();
			}*/

			/*if (players[i]->GetHasWon())
			{
				std::cout << "Win!\n";
				fade = true;
				winner_id = i;
				//for (auto& obstacle : entities)
				//{
					//obstacle->Destroy();
				//}
			}*/

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
		/*
		if ((*it)->GetType() == EntityType::OBSTACLE_WIN_CONDITION)
		{
			auto winCondition = static_cast<Obstacle*>(it->get());
			int winningPlayer = winCondition->GetWinningPlayerID();
			//std::cout << "Winner: Player: " << winningPlayer << "\n";
			if (winningPlayer != -1)
			{
				fade = true;
				winner_id = winningPlayer;
				//exit(0);
			}

			//exit(0);
		}*/

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
				if ((*it)->GetType() != EntityType::PARTICLE)
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
		for (int i = 0; i < players.size(); i++)
		{
			std::cout << "player " << i << " has: " << players[i]->points << "\n";
		}

		

		points_timer = std::chrono::high_resolution_clock::now() + std::chrono::seconds(1);
	}

	if (GameModule::input->GetKeyState(SDL_SCANCODE_F12))
	{
		players[activeplayerid]->points += 300;
	}

	if (gameplay)
	{
		//compute points on bar

		if (bar)
		{
			float x = default_bar_pos.x - point_shift * (players[activeplayerid]->points / point_per_change_bar);


			if (last_bar_pos.x != x && frame_rotating == false)
			{
				frame_rotating = true;
				start_rotation = frame->rotation;
			}

			if (frame_rotating)
			{
				frame->rotation -= 1.0f;
				bar->SetPosition(glm::vec3(bar->position.x - (float)point_shift / 22.0f, default_bar_pos.y, 0.0f));

				if (frame->rotation <= start_rotation - 22.0f)
				{
					last_bar_pos.x = x;
					frame_rotating = false;
				}
			}
		}
	}

	if (players.size() > 0)
	{
		camera.Translate(players[activeplayerid]->GetPosition() + glm::vec3(0, 10, 0));
		camera.Shake();

		//if(GameModule::input->GetKeyState(SDL_SCANCODE_B))
		//std::cout << "PL: " << players[activeplayerid]->GetPosition().x << " " << players[activeplayerid]->GetPosition().y << " " << players[activeplayerid]->GetPosition().z << "\n";
		//std::cout << "CAM: " << camera.GetPosition().x << " " << camera.GetPosition().y << " " << camera.GetPosition().z << "\n";

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
	if (activeplayerid != -1) //first iteration AccessViolation exception
		players[activeplayerid]->QuitShooting();

	++activeplayerid;
	activeplayerid %= players.size();

	std::string temp = "";
	if (players[activeplayerid]->currentcharacter == 0)
		temp += "pirate";
	else if (players[activeplayerid]->currentcharacter == 1)
		temp += "oldboy";
	else if (players[activeplayerid]->currentcharacter == 2)
		temp += "maskman";
	else
		temp += "lady";

	temp += std::to_string((rand() % GameModule::resources->GetIntParameter("quotes")));

	GameModule::audio->PlaySound(temp);

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


	float object_size = 4;

	int no_spawn_chance = GameModule::resources->GetIntParameter("no_spawn_chance");
	int light_spawn_chance = GameModule::resources->GetIntParameter("light_spawn_chance");
	int heavy_spawn_chance = GameModule::resources->GetIntParameter("heavy_spawn_chance");
	int expl_spawn_chance = GameModule::resources->GetIntParameter("expl_spawn_chance");
	int hint_spawn_chance = GameModule::resources->GetIntParameter("hint_spawn_chance");

	std::uniform_int_distribution<> random_spawner(0, (no_spawn_chance + light_spawn_chance + heavy_spawn_chance + expl_spawn_chance+ hint_spawn_chance));
	std::uniform_real_distribution<> random_position(-100, 100);
	std::uniform_real_distribution<> random_win_spawn(0.0f, 0.6f);

	glm::vec3 scale(1, 0.5, 1);
	glm::vec3 biggerScale(1, 0.1, 1);

	bool super_wide_spawned = false;
	bool win_spawned = false;
	int win_spawn_pos = std::floorf(random_win_spawn(GameModule::random_gen) * obstacles_amount_per_wall * obstacles_amount_per_wall *
		((1.0f * no_spawn_chance) / (light_spawn_chance + heavy_spawn_chance + expl_spawn_chance + no_spawn_chance + hint_spawn_chance)));


	static const double explosionRadius = GameModule::resources->GetIntParameter("explosion_radius");//3;

//	auto obj = std::make_shared<Obstacle>(EntityType::OBSTACLE_WIN_CONDITION, dynamic_world, glm::vec3(2,0,2), scale, explosionRadius);
//	obj->Init();
//	entities.push_back(obj);

	for (int i = 0; i <= obstacles_amount_per_wall; i++)
	{
		for (int j = 0; j <= obstacles_amount_per_wall; j++)
		{
			if (super_wide_spawned)
			{
				super_wide_spawned = false;
				continue;
			}

			bool is_position_near_player = false;

			float current_x = -80 + i * object_size;
			float current_z = -80 + j * object_size;

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
					if (obj->GetScale().x > 1.5)
						super_wide_spawned = true;
				}
				else if (rand_obstacle_type < expl_spawn_chance + heavy_spawn_chance + light_spawn_chance)
				{
					auto obj = std::make_shared<Obstacle>(EntityType::OBSTACLE_LIGHT, dynamic_world, pos, scale, explosionRadius);
					obj->Init();
					entities.push_back(obj);
					if (obj->GetScale().x > 1.5)
						super_wide_spawned = true;
				}
				else if (rand_obstacle_type < expl_spawn_chance + heavy_spawn_chance + light_spawn_chance + hint_spawn_chance)
				{
					auto obj = std::make_shared<Obstacle>(EntityType::HINT, dynamic_world, pos, scale, explosionRadius);
					obj->Init();
					entities.push_back(obj);
				}
				else
				{
					if (win_spawn_pos > 0)
						--win_spawn_pos;
					else if (!win_spawned)
					{
						auto obj = std::make_shared<Obstacle>(EntityType::OBSTACLE_WIN_CONDITION, dynamic_world, pos, scale, explosionRadius);
						obj->Init();
						entities.push_back(obj);
						std::cout << "Win spawned at " << pos.x << " " << pos.z << "\n";
						GameModule::winposx = pos.x;
						GameModule::winposy = pos.z;
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
	static const std::string playerNames[4] = { "player4", "player1" , "player2", "player3" };
	obstacle_data.delay = std::chrono::milliseconds(obstacle_data.default_delay);

	ResetDestructTimer();
	ResetTurnTimer();

	int object_size = 3; //lel - from lifes

	int limit = object_size * GameModule::resources->GetIntParameter("obstacles_amount") - 20;

	limit /= 2;

	for (int i = 0; i < GameModule::resources->GetIntParameter("playersamount"); i++)
	{
		AddPlayer(glm::vec3((i * limit) % (limit * 2), 0, ((i*limit)/(limit*2)) * limit), playerNames[i], i);
		//std::cout << i << ": " << (i * limit) % (limit * 2) << " " << ((i*limit) / (limit * 2)) * limit << "\n";
	}

	activeplayerid = 0;
	///NextPlayer(); //hack to init turntimer properly

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

void GameState::AddPlayer(glm::vec3 startpos, std::string name, int id)
{
	auto obj = std::make_shared<Ship>(dynamic_world, startpos, entities, name, id);
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

	float resolution_x = GameModule::resources->GetIntParameter("resolution_x");
	float resolution_y = GameModule::resources->GetIntParameter("resolution_y");


	res_ratio = resolution_x / resolution_y;
	
	glm::vec2 pos(0, 0);
	glm::vec2 size(0.5, 0.5 * res_ratio);

	players_graphics.push_back(std::make_shared<Mesh>("quad", "player_big1", pos, size));
	players_graphics.push_back(std::make_shared<Mesh>("quad", "player_big2", pos, size));
	players_graphics.push_back(std::make_shared<Mesh>("quad", "player_big3", pos, size));
	players_graphics.push_back(std::make_shared<Mesh>("quad", "player_big4", pos, size));



	pos.x = 0.8f;
	pos.y = -0.5f;
	size = glm::vec2(0.2, 0.2 * res_ratio);
	frame = std::make_shared<Mesh>("quad", "frame", pos, size);
	

	pos.x = 1.91f;
	pos.y = -0.84f;
	default_bar_pos = pos;
	last_bar_pos = pos;
	size = glm::vec2(1.3f, 0.025f * res_ratio);
	bars.push_back(std::make_shared<Mesh>("quad", "bar", pos, size));
	bars.push_back(std::make_shared<Mesh>("quad", "bar", pos, size));
	bars.push_back(std::make_shared<Mesh>("quad", "bar", pos, size));
	bars.push_back(std::make_shared<Mesh>("quad", "bar", pos, size));


	point_shift = 0.2f;


	pos.x = 0.8f;
	pos.y = -0.5f;
	size = glm::vec2(0.2, 0.2 * res_ratio);
	portraits.push_back(std::make_shared<Mesh>("quad", "portrait3", pos, size));
	portraits.push_back(std::make_shared<Mesh>("quad", "portrait", pos, size));
	portraits.push_back(std::make_shared<Mesh>("quad", "portrait2", pos, size));
	portraits.push_back(std::make_shared<Mesh>("quad", "portrait1", pos, size));



	pos.x = 0;
	pos.y = 0;
	size = glm::vec2(1, 1);

	gui.push_back(std::make_shared<Mesh>("quad", "start", pos, size));
	/*

	pos.x = 0;
	pos.y = 0;
	size = glm::vec2(1, 1.3);
	gui.push_back(std::make_shared<Mesh>("quad", "start_background", pos, size));
	*/
}

void GameState::ShowNextPlayer(bool show, int player_id)
{
	if (show)
	{
		//must add player_id

		next_player = players_graphics[player_id];
		portrait = nullptr;
		
		static bool must_init_win = true;
		if (winner_id != -1 && must_init_win)
		{
			must_init_win = false;
			glm::vec2 pos(0, 0);
			glm::vec2 size(0.8, 1.05);

			gui.push_back(std::make_shared<Mesh>("quad", "win3", pos, size));
		}
	}
	else
	{
		next_player = nullptr;
		portrait = portraits[player_id];	
		bar = bars[player_id];
		last_bar_pos.x = default_bar_pos.x - point_shift * (players[activeplayerid]->points / point_per_change_bar);
		start_rotation = bar->rotation;
	}
}

void GameState::WinScreen()
{
	//next_player = players_graphics[winner_id];
//	gui.push_back(std::make_shared<Mesh>("quad", "win3", pos, size));

	if (winner_id > 0)
		winner_id--;
	else
		winner_id = players.size() - 1;

	ShowNextPlayer(true, winner_id);
}