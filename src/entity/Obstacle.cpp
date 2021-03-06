#include "Obstacle.h"

Obstacle::Obstacle(EntityType obj_type, std::shared_ptr<btDiscreteDynamicsWorld> world_ptr, glm::vec3 start_pos, glm::vec3 scale, double initWaveRadius)
	: Entity(world_ptr, start_pos, scale, initWaveRadius), spawnTime{ std::chrono::high_resolution_clock::now() }, winningPlayerID{ -1 }
{
	glm::vec3 biggerScale(1.9, 0.5, 1);
	type = obj_type;

	if (type == EntityType::OBSTACLE_HEAVY)
	{
		points = 50;

		mesh = GameModule::resources->GetMesh("wall");
		this->scale.x = 1.6;
		this->scale.z = 1.6;
	}
	else if (type == EntityType::HINT)
	{
		points = 0;
		mesh = GameModule::resources->GetMesh("obstacle_hint");
	}
	else if (type == EntityType::OBSTACLE_LIGHT)
	{
		points = 10;

		std::uniform_int_distribution<int> random(0, 8);
		int number = random(GameModule::random_gen);
		if (number < 2)
		{
			mesh = GameModule::resources->GetMesh("obstacle_light");
			this->scale = glm::vec3(1, 0.1, 1);
		}
		else if (number < 4)
		{
			mesh = GameModule::resources->GetMesh("obstacle_light_1");
			this->scale = glm::vec3(1, 0.1, 1);
		}
		else if (number == 4)
		{
			mesh = GameModule::resources->GetMesh("obstacle_light_2");
			this->scale = biggerScale;// glm::vec3(1.2, 0.1, 0.6);
		}
		else if (number == 5)
		{
			mesh = GameModule::resources->GetMesh("obstacle_light_3");
			this->scale = biggerScale;//glm::vec3(1.2, 0.1, 0.6);
		}
		else if (number == 6)
		{
			mesh = GameModule::resources->GetMesh("obstacle_light_5");
			this->scale = biggerScale;//glm::vec3(1.2, 0.1, 0.6);
		}
		else if (number > 6)
		{
			mesh = GameModule::resources->GetMesh("obstacle_light_4");
		}
	}
	else if (type == EntityType::OBSTACLE_EXPLOSIVE)
	{
		points = 20;
		//mesh = GameModule::resources->GetMesh("obstacle_explosive");

		std::uniform_int_distribution<int> random(0, 1);
		int number = random(GameModule::random_gen);
		if (number == 0)
			mesh = GameModule::resources->GetMesh("obstacle_explosive");
		else
		{
			mesh = GameModule::resources->GetMesh("obstacle_explosive_1");
			this->scale = glm::vec3(1.5, 0.1, 1.5);
		}

	}
	else if (type == EntityType::PARTICLE)
	{
		points = 0;
		mesh = GameModule::resources->GetMesh("particle");
	}
	else if (type == EntityType::EXPLOSION)
	{
		points = 0;
		mesh = GameModule::resources->GetMesh("sphere");
	}
	else if (type == EntityType::OBSTACLE_WIN_CONDITION)
	{
		points = GameModule::resources->GetIntParameter("treasurepoints");
		mesh = GameModule::resources->GetMesh("FIND IT");
	}
}

void Obstacle::Init()
{
	physic_body = std::make_unique<PhysicBody>(world.lock(), pos, scale, type, shared_from_this());


	if (type == EntityType::PARTICLE)
	{

	}
	else if (type == EntityType::EXPLOSION)
	{
		physic_body->body->setCollisionFlags(physic_body->body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	}
	else if (type == EntityType::OBSTACLE_HEAVY || type == EntityType::OBSTACLE_WIN_CONDITION || type == EntityType::HINT)
	{
		physic_body->body->setLinearFactor(btVector3(0, 0, 0));
	}
	else
	{
		//2d movement
		physic_body->body->setLinearFactor(btVector3(1, 0, 1));
	}

	if (GameModule::resources->GetBoolParameter("obstaclerotation"))
	{
		if (type != EntityType::OBSTACLE_HEAVY && type != EntityType::HINT)
			physic_body->body->setAngularFactor(btVector3(0, 1, 0));
		else
			physic_body->body->setAngularFactor(btVector3(0, 0, 0));
	}
	else
	{
		physic_body->body->setAngularFactor(btVector3(0, 0, 0));
	}

	physic_body->body->activate(true);

	//to avoid render on start in world center
	transform_mat = physic_body->GetTransformMatrix();
	Rotate(glm::radians(90.0));

	physic_body->body->setDamping(0.5f, 0.5f);
}

void Obstacle::ShowWay()
{
	if (type == EntityType::HINT)
	{
		//std::cout << pos.x << " " <<pos.z <<  " "<< GameModule::winposx << " "<< GameModule::winposy << "\n";

		mesh = GameModule::resources->GetMesh("obstacle_hint_1");
		glm::vec2 winpos = glm::vec2(GameModule::winposy - pos.z, GameModule::winposx -pos.x); //Sry ._.
		float newAngle = (atan2((-winpos.x), ((winpos.y))) -glm::radians(90.0f));

		Rotate(newAngle);
	}
}

void Obstacle::Update()
{
	transform_mat = physic_body->GetTransformMatrix();

	//static std::chrono::high_resolution_clock::time_point restart_timer = std::chrono::high_resolution_clock::now();
	if (type == EntityType::EXPLOSION)
	{
		if (std::chrono::high_resolution_clock::now() > spawnTime + std::chrono::milliseconds(100))
			Destroy();
		else
		{

		}
	}
	else if (type == EntityType::PARTICLE)
	{
		if (std::chrono::high_resolution_clock::now() > spawnTime +
			std::chrono::milliseconds(GameModule::resources->GetIntParameter("particlelifetime")))
			Destroy();
		else
		{

		}
	}

}
