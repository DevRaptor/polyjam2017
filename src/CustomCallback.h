#pragma once

#include <iostream>

#include <btBulletDynamicsCommon.h>

#include "entity/PhysicBody.h"
#include "entity/Ship.h"

struct CustomCallback : public btCollisionWorld::ContactResultCallback 
{
	btScalar addSingleResult(btManifoldPoint& cp,
		const btCollisionObjectWrapper* obj0, int, int,
		const btCollisionObjectWrapper* obj1, int, int) override
	{
		const RigidBody* own_obj0 = static_cast<const RigidBody*>(obj0->getCollisionObject());
		const RigidBody* own_obj1 = static_cast<const RigidBody*>(obj1->getCollisionObject());

		//bullet can collide only with meteor, so must add point
		
		if (own_obj0->GetType() != EntityType::BULLET
			&& own_obj1->GetType() != EntityType::BULLET)
		{
			return 0;
		}

		if (own_obj0->GetType()  == EntityType::BULLET)
			Ship::points++;
		else if(own_obj1->GetType() == EntityType::BULLET)
			Ship::points++;

		own_obj0->GetOwner()->Destroy();
		own_obj1->GetOwner()->Destroy();

		return 0; // default value
	}
};