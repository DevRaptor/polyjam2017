#include "Obstacle.h"

Obstacle::Obstacle(EntityType obj_type, std::shared_ptr<btDiscreteDynamicsWorld> world_ptr, glm::vec3 start_pos, glm::vec3 scale, double initWaveRadius)
	: Entity(world_ptr, start_pos, scale, initWaveRadius), spawnTime{ std::chrono::high_resolution_clock::now() }
{
	
	type = obj_type;

	if(type == EntityType::OBSTACLE_HEAVY)
		mesh = GameModule::resources->GetMesh("teapot");
	else if (type == EntityType::OBSTACLE_LIGHT)
		mesh = GameModule::resources->GetMesh("teapot");
	else if (type == EntityType::OBSTACLE_EXPLOSIVE)
		mesh = GameModule::resources->GetMesh("teapot");
	else if (type == EntityType::PARTICLE)
		mesh = GameModule::resources->GetMesh("particle");
	else if (type == EntityType::EXPLOSION)
		mesh = GameModule::resources->GetMesh("sphere");
		
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
	else
	{
		//2d movement
		physic_body->body->setLinearFactor(btVector3(1, 0, 1));
	}
	
	physic_body->body->activate(true);
	
	//to avoid render on start in world center
	transform_mat = physic_body->GetTransformMatrix();
}

void Obstacle::Update()
{
	transform_mat = physic_body->GetTransformMatrix();

	//static std::chrono::high_resolution_clock::time_point restart_timer = std::chrono::high_resolution_clock::now();
	if (type == EntityType::EXPLOSION)
	{
		if(std::chrono::high_resolution_clock::now() > spawnTime + std::chrono::milliseconds(100))
			Destroy();
		else
		{

		}
	}
	else if (type == EntityType::PARTICLE)
	{
		if (std::chrono::high_resolution_clock::now() > spawnTime + std::chrono::milliseconds(2000))
			Destroy();
		else
		{

		}
	}

}
