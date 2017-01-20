#include "Ship.h"

#include "Bullet.h"

int Ship::points = 0;

Ship::Ship(std::shared_ptr<btDiscreteDynamicsWorld> world_ptr, glm::vec3 start_pos,
	std::vector<std::shared_ptr<Entity>>& bullet_container)
	: Entity(world_ptr, start_pos, glm::vec3(1.0f, 1.0f, 1.0f)), bullets(bullet_container)
{
	type = EntityType::SHIP;
	mesh = GameModule::resources->GetMesh("cube");
	
	points = 0;

	move_speed = GameModule::resources->GetFloatParameter("ship_move_speed");
	move_damping = GameModule::resources->GetFloatParameter("ship_move_damping");
	stop_damping = GameModule::resources->GetFloatParameter("ship_stop_damping");

	move_speed_max = GameModule::resources->GetFloatParameter("ship_move_speed_max");

	// min/max world position
	movement_limit = (GameModule::resources->GetFloatParameter("camera_pos_y") / 2.0f) * 0.8f;

	shoot_delay = std::chrono::milliseconds(GameModule::resources->GetIntParameter("ship_shoot_delay"));
}

void Ship::Init()
{
	physic_body = std::make_unique<PhysicBody>(world.lock(), pos, glm::vec3(1.0f, 1.0f, 1.0f), type, shared_from_this());

	//only left/right movement
	physic_body->body->setLinearFactor(btVector3(0, 0, 1));

	//disable rotation
	physic_body->body->setAngularFactor(btVector3(0, 0, 0));

}

void Ship::Update()
{
	transform_mat = physic_body->GetTransformMatrix();

	btTransform transform;
	physic_body->body->getMotionState()->getWorldTransform(transform);
	btVector3 pos = transform.getOrigin();

	if (pos.getZ() < movement_limit &&
		(GameModule::input->GetKeyState(SDL_SCANCODE_LEFT)
			|| GameModule::input->GetKeyState(SDL_SCANCODE_A)))
	{
		physic_body->body->activate(true);

		btVector3 impulse(0.0f, 0.0f, move_speed);
		physic_body->body->applyCentralImpulse(impulse);

		physic_body->body->setDamping(move_damping, 0);
	}
	else if (pos.getZ() > -movement_limit &&
		(GameModule::input->GetKeyState(SDL_SCANCODE_RIGHT)
			|| GameModule::input->GetKeyState(SDL_SCANCODE_D)))
	{
		physic_body->body->activate(true);

		btVector3 impulse(0.0f, 0.0f, -move_speed);
		physic_body->body->applyCentralImpulse(impulse);
		physic_body->body->setDamping(move_damping, 0);
	}
	else
	{
		physic_body->body->setDamping(stop_damping, 0);
	}

	btVector3 velocity = physic_body->body->getLinearVelocity();
	if (velocity.length() > move_speed_max)
	{
		velocity *= move_speed_max / velocity.length();
		physic_body->body->setLinearVelocity(velocity);
	}


	//shooting
	if ((std::chrono::high_resolution_clock::now() > shoot_timer)
		&& GameModule::input->GetKeyState(SDL_SCANCODE_SPACE))
	{
		Shoot();

		shoot_timer = std::chrono::high_resolution_clock::now() + shoot_delay;
	}
}

void Ship::Shoot()
{
	btTransform transform;
	physic_body->body->getMotionState()->getWorldTransform(transform);

	btVector3 vec = transform.getOrigin();
	vec.setX(vec.getX() - 2.0f);
	glm::vec3 pos(vec.getX(), vec.getY(), vec.getZ());

	auto bullet = std::make_shared<Bullet>(world.lock(), pos, glm::vec3(0.3f, 0.3f, 0.3f));
	bullet->Init();

	bullets.push_back(bullet);
}
