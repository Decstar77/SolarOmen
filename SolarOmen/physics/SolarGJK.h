#pragma once

#include "SolarBody.h"
namespace cm
{
	bool GJKIntersection(RigidBody* bodyA, RigidBody* bodyB);
	bool EPAIntersection(RigidBody* bodyA, RigidBody* bodyB, Vec3f* ptOnA, Vec3f* ptOnB);
}
