#pragma once

#include <iostream>
#include <memory>

#include <btBulletDynamicsCommon.h>

#include "rendering/Mesh.h"
#include "modules/GameModule.h"
#include "entity/PhysicBody.h"

//EntityType declaration in PhysicBody

class Entity
{
public:
	glm::mat4 transform_mat;

	Entity(std::shared_ptr<btDiscreteDynamicsWorld> world_ptr,
		glm::vec3 start_pos, glm::vec3 init_scale)
		: world(world_ptr), pos(start_pos), scale(init_scale),
		destroyed(false)
	{
		type = EntityType::NONE;
	}

	virtual ~Entity() = default;

	virtual void Init() = 0;
	virtual void Update() = 0;

	virtual void DoShoot() {}
	virtual void Move(btVector3* direction) {}
	void Rotate(double angleInDegrees)
	{
		btQuaternion qNewOrientation;
		//qNewOrientation.setEuler(angleInDegrees, 0, 0);
		//qNewOrientation.setEuler(glm::radians(angleInDegrees), 0, 0);
		qNewOrientation.setRotation(btVector3(0,1,0), angleInDegrees);
		btTransform transBody;
		physic_body->body->getMotionState()->getWorldTransform(transBody);
		transBody.setRotation(qNewOrientation);
		physic_body->body->setCenterOfMassTransform(transBody);
	}

	void Draw()
	{
		if (mesh)
		{
			mesh->Draw();
		}
	}

	EntityType GetType() { return type; }

	void Destroy() { destroyed = true; }
	bool IsDestroyed() { return destroyed; }

	RigidBody* GetRigidBody() { return physic_body->body.get(); }
	glm::vec3 GetPosition() 
	{
		btTransform transform;
		physic_body->body->getMotionState()->getWorldTransform(transform);
		//float pos_x = transform.getOrigin().getX();
		return glm::vec3(transform.getOrigin().getX(), transform.getOrigin().getY(), transform.getOrigin().getZ());
	}

protected:
	EntityType type;

	std::shared_ptr<Mesh> mesh;
	std::unique_ptr<PhysicBody> physic_body;
	std::weak_ptr<btDiscreteDynamicsWorld> world;
	glm::vec3 pos;
	glm::vec3 scale;
	bool destroyed;
};
