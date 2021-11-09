#pragma once
#include "SolarRigidBodyMK2.h"

namespace cm::MK2
{
	bool GJKIntersection(RigidBody* bodyA, RigidBody* bodyB);
	bool GJKIntersection(RigidBody* bodyA, RigidBody* bodyB, const float bias, Vec3f* ptOnA, Vec3f* ptOnB);
	void GJKClosestPoints(RigidBody* bodyA, RigidBody* bodyB, Vec3f* ptOnA, Vec3f* ptOnB);
}