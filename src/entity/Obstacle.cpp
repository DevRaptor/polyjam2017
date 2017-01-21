#include "Obstacle.h"

Obstacle::Obstacle(std::shared_ptr<btDiscreteDynamicsWorld> world_ptr, glm::vec3 start_pos, glm::vec3 scale)
	: Entity(world_ptr, start_pos, scale)
{
	type = EntityType::OBSTACLE;
	mesh = GameModule::resources->GetMesh("teapot");
}

void Obstacle::Init()
{
	physic_body = std::make_unique<PhysicBody>(world.lock(), pos, scale, type, shared_from_this());

	//2d movement
	physic_body->body->setLinearFactor(btVector3(1, 0, 1));

	physic_body->body->activate(true);

	//to avoid render on start in world center
	transform_mat = physic_body->GetTransformMatrix();
}

void Obstacle::Update()
{

}
