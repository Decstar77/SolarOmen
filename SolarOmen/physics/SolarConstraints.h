#pragma once
#include "../core/SolarCore.h"
#include "SolarMatrix.h"
#include "SolarBody.h"

namespace cm
{
	class DistanceConstraint
	{
	public:
		RigidBody* bodyA;
		RigidBody* bodyB;

		Vec3f anchorA;		// @NOTE: The anchor location in bodyA's space
		Vec3f axisA;		// @NOTE: The axis direction in bodyA's space
		Vec3f anchorB;		// @NOTE: The anchor location in bodyB's space
		Vec3f axisB;		// @NOTE: The axis direction in bodyB's space

	public:
		void PreSolve(const real32 dt);
		void Solve();
		void PostSolve();

		DistanceConstraint() :
			cachedLambda(1),
			jacobian(1, 12),
			bodyA(nullptr),
			bodyB(nullptr)
		{
			cachedLambda.Zero();
			baumgarte = 0.0f;
		}

	private:
		MatMN jacobian;
		VecN cachedLambda;
		real32 baumgarte;
	};

	class PenetrationConstraint
	{
	public:
		RigidBody* bodyA;
		RigidBody* bodyB;

		Vec3f anchorA;		// @NOTE: The anchor location in bodyA's space
		Vec3f axisA;		// @NOTE: The axis direction in bodyA's space
		Vec3f anchorB;		// @NOTE: The anchor location in bodyB's space
		Vec3f axisB;		// @NOTE: The axis direction in bodyB's space

		Vec3f normalA;		// @NOTE: in Body A's local space
		VecN cachedLambda;
		MatMN jacobian;
		real32 baumgarte;
		real32 friction;

	public:
		PenetrationConstraint() :
			cachedLambda(3), jacobian(3, 12),
			bodyA(nullptr), bodyB(nullptr)
		{
			cachedLambda.Zero();
			baumgarte = 0.0f;
			friction = 0.0f;
		}

		void PreSolve(const float dt_sec);
		void Solve();
		void PostSolve();
	};
}
