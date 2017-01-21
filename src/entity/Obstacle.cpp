#include "Obstacle.h"

Obstacle::Obstacle(EntityType obj_type, std::shared_ptr<btDiscreteDynamicsWorld> world_ptr, glm::vec3 start_pos, glm::vec3 scale)
	: Entity(world_ptr, start_pos, scale)
{
	
	type = obj_type;

	if(type == EntityType::OBSTACLE)
		mesh = GameModule::resources->GetMesh("teapot");
	else if(type == EntityType::PARTICLE)
		mesh = GameModule::resources->GetMesh("particle");

	points = 5; //delete it after adding types
	switch (type) //points range on every
	{
	case EntityType::SHIP:
		break;
	case EntityType::OBSTACLE:
		break;
	case EntityType::PARTICLE:
		break;
	case EntityType::BULLET:
		break;
	case EntityType::NONE:
		break;
	default:
		break;
	}

}

void Obstacle::Init()
{
	physic_body = std::make_unique<PhysicBody>(world.lock(), pos, scale, type, shared_from_this());
	
	if (type == EntityType::OBSTACLE)
	{
		//2d movement
		physic_body->body->setLinearFactor(btVector3(1, 0, 1));
	}
	else if (type == EntityType::PARTICLE)
	{

	}
	
	physic_body->body->activate(true);
	
	//to avoid render on start in world center
	transform_mat = physic_body->GetTransformMatrix();
}

void Obstacle::Update()
{
	transform_mat = physic_body->GetTransformMatrix();
}
