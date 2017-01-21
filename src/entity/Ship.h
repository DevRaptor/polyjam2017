#pragma once

#include <chrono>
#include <vector>

#include "entity/Entity.h"

class Ship : public Entity, public std::enable_shared_from_this<Ship>
{
public:
	static int points; //reset in constructor

	Ship(std::shared_ptr<btDiscreteDynamicsWorld> world_ptr, glm::vec3 start_pos,
		std::vector<std::shared_ptr<Entity>>& bullet_container);

	void Init() override;
	void Update() override;

	void DoShoot() override;
	void Move(btVector3* direction) override;

protected:
	float move_speed;
	float move_speed_max;

	float move_damping;
	float stop_damping;

	float movement_limit;

	std::chrono::milliseconds shoot_delay; //time to next shoot
	std::chrono::high_resolution_clock::time_point shoot_timer;

	std::vector<std::shared_ptr<Entity>>& bullets;

	void Shoot();
};

