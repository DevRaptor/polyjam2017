#pragma once

#include <chrono>
#include <vector>

#include "entity/Entity.h"

class Ship : public Entity, public std::enable_shared_from_this<Ship>
{
public:
	static int character;
	int currentcharacter;

	Ship(std::shared_ptr<btDiscreteDynamicsWorld> world_ptr, glm::vec3 start_pos,
		std::vector<std::shared_ptr<Entity>>& bullet_container, std::string name, int playerID);

	void Init() override;
	void Update() override;

	void DoShoot() override;
	void Move(btVector3* direction) override;
	

	bool shot = false;
	bool AlreadyShot() override;
	void QuitShooting() override;

	void SetIsEnabled(bool isTrue) { isEnabled = isTrue; }
	bool GetIsEnabled() { return isEnabled; }

	void SetHasWon(bool isTrue) { hasWon = isTrue; }
	bool GetHasWon() { return hasWon; }
protected:
	float move_speed;
	float move_speed_max;

	float move_damping;
	float stop_damping;

	float movement_limit;

	float angle;
	bool isEnabled;
	bool hasWon;
	int id;

	std::chrono::milliseconds shoot_delay; //time to next shoot
	std::chrono::high_resolution_clock::time_point shoot_timer;

	std::vector<std::shared_ptr<Entity>>& bullets;

	void Shoot();
};

