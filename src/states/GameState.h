#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include <SDL.h>

#include <btBulletDynamicsCommon.h>

#include "CustomCallback.h"
#include "entity/Entity.h"
#include "rendering/Camera.h"

class Renderer;

class GameState
{
	friend class Renderer;

public:
	GameState();
	~GameState();

	void Update(std::chrono::milliseconds delta_time);

private:
	std::vector<std::shared_ptr<Entity>> players;
	std::vector<std::shared_ptr<Entity>> entities;

	//physics
	std::unique_ptr<btBroadphaseInterface> broad_phase;
	std::unique_ptr<btDefaultCollisionConfiguration> collision_config;
	std::unique_ptr<btCollisionDispatcher> dispatcher;
	std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
	std::shared_ptr<btDiscreteDynamicsWorld> dynamic_world;

	Camera camera;

	CustomCallback callback;


	struct
	{
		float pos_x;
		float pos_z;

		float scale_min;
		float scale_max;
		float distortion;

		int min_delay; //in ms
		int default_delay; //in ms
		std::chrono::milliseconds delay; //time to next shoot
		std::chrono::high_resolution_clock::time_point timer;
	} meteor_data;

	void SpawnMeteor();

	void InitGameplay();
	void RestartGameplay();
};
