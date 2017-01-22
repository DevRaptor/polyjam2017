#pragma once

#include <iostream>

#include <btBulletDynamicsCommon.h>

#include "entity/PhysicBody.h"
#include "entity/Ship.h"
#include "entity/Obstacle.h"
#include "entity/Bullet.h"

struct CustomCallback : public btCollisionWorld::ContactResultCallback 
{
	btScalar addSingleResult(btManifoldPoint& cp,
		const btCollisionObjectWrapper* obj0, int, int,
		const btCollisionObjectWrapper* obj1, int, int) override
	{
		const RigidBody* own_obj0 = static_cast<const RigidBody*>(obj0->getCollisionObject());
		const RigidBody* own_obj1 = static_cast<const RigidBody*>(obj1->getCollisionObject());

		//bullet can collide only with meteor, so must add point
		
		if (own_obj0->GetType() == EntityType::BULLET && own_obj1->GetType() == EntityType::OBSTACLE_WIN_CONDITION)
		{
			//static_cast<Ship*>(own_obj0->GetOwner().get())->SetHasWon(true);
			auto object = static_cast<Obstacle*>(own_obj1->GetOwner().get());
			auto bullet = static_cast<Bullet*>(own_obj0->GetOwner().get());
			object->SetWinningPlayerID(bullet->GetPlayerID());
			return 0;
		}
		else if (own_obj1->GetType() == EntityType::BULLET && own_obj0->GetType() == EntityType::OBSTACLE_WIN_CONDITION)
		{
			//static_cast<Ship*>(own_obj1->GetOwner().get())->SetHasWon(true);
			auto object = static_cast<Obstacle*>(own_obj0->GetOwner().get());
			auto bullet = static_cast<Bullet*>(own_obj1->GetOwner().get());

			object->SetWinningPlayerID(bullet->GetPlayerID());
			return 0;
		}

		if (own_obj0->GetType() == EntityType::BULLET && own_obj1->GetType() == EntityType::OBSTACLE_WIN_CONDITION)
		{
			own_obj0->GetOwner()->Destroy();
			return 0;
		}
		else if (own_obj1->GetType() == EntityType::BULLET && own_obj0->GetType() == EntityType::OBSTACLE_WIN_CONDITION)
		{
			own_obj1->GetOwner()->Destroy();
			return 0;
		}

		if (own_obj0->GetType() != EntityType::BULLET
			&& own_obj1->GetType() != EntityType::BULLET)
		{
			return 0;
		}

		if (own_obj0->GetType() == EntityType::SHIP && own_obj1->GetType() == EntityType::BULLET)
		{
			static_cast<Ship*>(own_obj0->GetOwner().get())->SetIsEnabled(false);
			own_obj1->GetOwner()->Destroy();
			return 0;
		}
		else if (own_obj1->GetType() == EntityType::SHIP && own_obj0->GetType() == EntityType::BULLET)
		{
			static_cast<Ship*>(own_obj1->GetOwner().get())->SetIsEnabled(false);
			own_obj0->GetOwner()->Destroy();
			return 0;
		}

		

		own_obj0->GetOwner()->Destroy();
		own_obj1->GetOwner()->Destroy();

		return 0; // default value
	}
};