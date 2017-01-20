#include "Meteor.h"

Meteor::Meteor(std::shared_ptr<btDiscreteDynamicsWorld> world_ptr, glm::vec3 start_pos, glm::vec3 scale)
	: Entity(world_ptr, start_pos, scale)
{
	type = EntityType::METEOR;
	mesh = GameModule::resources->GetMesh("cube");
}

void Meteor::Init()
{
	physic_body = std::make_unique<PhysicBody>(world.lock(), pos, scale, type, shared_from_this());

	//2d movement
	physic_body->body->setLinearFactor(btVector3(1, 0, 1));


	float move_speed_min = GameModule::resources->GetFloatParameter("meteor_move_speed_min");
	float move_speed_max = GameModule::resources->GetFloatParameter("meteor_move_speed_max");
	std::uniform_real_distribution<> rand_vel(move_speed_min, move_speed_max);

	float deflection = GameModule::resources->GetFloatParameter("meteor_move_deflection");
	std::uniform_real_distribution<> rand_deflection(-deflection, deflection);

	btVector3 velocity = btVector3(rand_vel(GameModule::random_gen), 0, rand_deflection(GameModule::random_gen));


	float max_rotation = GameModule::resources->GetFloatParameter("meteor_max_rotation");
	std::uniform_real_distribution<> rand_rot(-max_rotation, max_rotation);

	btVector3 rotation = btVector3(rand_rot(GameModule::random_gen), rand_rot(GameModule::random_gen),
		rand_rot(GameModule::random_gen));


	physic_body->body->activate(true);

	physic_body->body->setLinearVelocity(velocity);
	physic_body->body->setAngularVelocity(rotation);

	//to avoid render on start in world center
	transform_mat = physic_body->GetTransformMatrix();
}

void Meteor::Update()
{
	transform_mat = physic_body->GetTransformMatrix();


	btTransform transform;
	physic_body->body->getMotionState()->getWorldTransform(transform);
	float pos_x = transform.getOrigin().getX();

	if (pos_x > 3.0f)
		Destroy();
}
