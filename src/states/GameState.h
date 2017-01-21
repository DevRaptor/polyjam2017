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

	int activeplayerid;
	//playersamount is ship::indexer;

	glm::mat4 floor_transform;
	std::shared_ptr<Mesh> floor_mesh;
	std::shared_ptr<PhysicBody> floor_physic_body;

	std::shared_ptr<btCollisionShape> groundShape;
	std::shared_ptr<btDefaultMotionState> groundMotionState;
	std::shared_ptr<btRigidBody> groundRigidBody;

	void AddFloor();

	void AddPlayer(glm::vec3 startpos);

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
	} obstacle_data;

	void SpawnObstacles();
	void CheckTriggers();

	void InitGameplay();
	void RestartGameplay();

	void Explosion(btVector3& pos);
};
