#pragma once
#include "core/SolarCore.h"
#include "SolarRigidBodyMK2.h"

namespace cm::MK2
{
	struct Contact {
		Vec3f ptOnA_WorldSpace;
		Vec3f ptOnB_WorldSpace;
		Vec3f ptOnA_LocalSpace;
		Vec3f ptOnB_LocalSpace;

		Vec3f normal;	// In World Space coordinates
		real32 separationDistance;	// positive when non-penetrating, negative when penetrating
		real32 timeOfImpact;

		RigidBody* bodyA;
		RigidBody* bodyB;

		void Resolve();
	};
}

