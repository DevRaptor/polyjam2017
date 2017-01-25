#pragma once

#include "entity/Entity.h"
#include <chrono>

class Obstacle : public Entity, public std::enable_shared_from_this<Obstacle>
{
public:
	Obstacle(EntityType obj_type, std::shared_ptr<btDiscreteDynamicsWorld> world_ptr,
		glm::vec3 start_pos, glm::vec3 scale, double initWaveRadius);

	void Init() override;
	void Update() override;

	void SetWinningPlayerID(int id) { winningPlayerID = id; }
	int GetWinningPlayerID() { return winningPlayerID; }

	void ShowWay();

private:
	std::chrono::high_resolution_clock::time_point spawnTime;
	int winningPlayerID;
};

