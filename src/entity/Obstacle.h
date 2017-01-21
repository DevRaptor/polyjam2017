#pragma once

#include "entity/Entity.h"

class Obstacle : public Entity, public std::enable_shared_from_this<Obstacle>
{
public:
	Obstacle(EntityType obj_type, std::shared_ptr<btDiscreteDynamicsWorld> world_ptr,
		glm::vec3 start_pos, glm::vec3 scale);

	int points;

	void Init() override;
	void Update() override;
};

