#include "SolarConstraints.h"

namespace cm
{
	static MatMN GetInverseMassMatrix(RigidBody* bodyA, RigidBody* bodyB)
	{
		MatMN invMassMatrix = MatMN(12, 12);
		invMassMatrix.Zero();

		invMassMatrix.rows[0][0] = bodyA->invMass;
		invMassMatrix.rows[1][1] = bodyA->invMass;
		invMassMatrix.rows[2][2] = bodyA->invMass;

		Mat3f invInertiaA = bodyA->GetInverseInertiaTensorWorldSpace();
		for (int32 i = 0; i < 3; i++)
		{
			invMassMatrix.rows[3 + i][3 + 0] = invInertiaA[i][0];
			invMassMatrix.rows[3 + i][3 + 1] = invInertiaA[i][1];
			invMassMatrix.rows[3 + i][3 + 2] = invInertiaA[i][2];
		}

		invMassMatrix.rows[6][6] = bodyB->invMass;
		invMassMatrix.rows[7][7] = bodyB->invMass;
		invMassMatrix.rows[8][8] = bodyB->invMass;

		Mat3f invInertiaB = bodyB->GetInverseInertiaTensorWorldSpace();
		for (int32 i = 0; i < 3; i++)
		{
			invMassMatrix.rows[9 + i][9 + 0] = invInertiaB[i][0];
			invMassMatrix.rows[9 + i][9 + 1] = invInertiaB[i][1];
			invMassMatrix.rows[9 + i][9 + 2] = invInertiaB[i][2];
		}

		return invMassMatrix;

	}

	static VecN GetVelocities(RigidBody* bodyA, RigidBody* bodyB)
	{
		VecN q_dt = VecN(12);

		q_dt[0] = bodyA->linearVelocity.x;
		q_dt[1] = bodyA->linearVelocity.y;
		q_dt[2] = bodyA->linearVelocity.z;

		q_dt[3] = bodyA->angularVelocity.x;
		q_dt[4] = bodyA->angularVelocity.y;
		q_dt[5] = bodyA->angularVelocity.z;

		q_dt[6] = bodyB->linearVelocity.x;
		q_dt[7] = bodyB->linearVelocity.y;
		q_dt[8] = bodyB->linearVelocity.z;

		q_dt[9] = bodyB->angularVelocity.x;
		q_dt[10] = bodyB->angularVelocity.y;
		q_dt[11] = bodyB->angularVelocity.z;

		return q_dt;
	}

	static void ApplyImpulses(const VecN& impulses, RigidBody* bodyA, RigidBody* bodyB)
	{
		Vec3f forceInternalA = Vec3f(0.0f);
		Vec3f torqueInternalA = Vec3f(0.0f);
		Vec3f forceInternalB = Vec3f(0.0f);
		Vec3f torqueInternalB = Vec3f(0.0f);

		forceInternalA[0] = impulses[0];
		forceInternalA[1] = impulses[1];
		forceInternalA[2] = impulses[2];

		torqueInternalA[0] = impulses[3];
		torqueInternalA[1] = impulses[4];
		torqueInternalA[2] = impulses[5];

		forceInternalB[0] = impulses[6];
		forceInternalB[1] = impulses[7];
		forceInternalB[2] = impulses[8];

		torqueInternalB[0] = impulses[9];
		torqueInternalB[1] = impulses[10];
		torqueInternalB[2] = impulses[11];

		bodyA->ApplyImpulseLinear(forceInternalA);
		bodyA->ApplyImpulseAngular(torqueInternalA);

		bodyB->ApplyImpulseLinear(forceInternalB);
		bodyB->ApplyImpulseAngular(torqueInternalB);
	}

	void DistanceConstraint::PreSolve(const real32 dt_sec)
	{
		// @NOTE: Get the world space position of the hinge from A's orientation
		Vec3f worldAnchorA = bodyA->BodySpaceToWorldSpace(anchorA);
		// @NOTE: Get the world space position of the hinge from B's orientation
		Vec3f worldAnchorB = bodyB->BodySpaceToWorldSpace(anchorB);

		Vec3f r = worldAnchorB - worldAnchorA;
		Vec3f ra = worldAnchorA - bodyA->GetCenterOfMassWorldSpace();
		Vec3f rb = worldAnchorB - bodyB->GetCenterOfMassWorldSpace();
		Vec3f a = worldAnchorA;
		Vec3f b = worldAnchorB;

		jacobian.Zero();

		Vec3f J1 = (a - b) * 2.0f;
		jacobian.rows[0][0] = J1.x;
		jacobian.rows[0][1] = J1.y;
		jacobian.rows[0][2] = J1.z;

		Vec3f J2 = Cross(ra, (a - b) * 2.0f);
		jacobian.rows[0][3] = J2.x;
		jacobian.rows[0][4] = J2.y;
		jacobian.rows[0][5] = J2.z;

		Vec3f J3 = (b - a) * 2.0f;
		jacobian.rows[0][6] = J3.x;
		jacobian.rows[0][7] = J3.y;
		jacobian.rows[0][8] = J3.z;

		Vec3f J4 = Cross(rb, (b - a) * 2.0f);
		jacobian.rows[0][9] = J4.x;
		jacobian.rows[0][10] = J4.y;
		jacobian.rows[0][11] = J4.z;

		// @NOTE: Apply warm starting from last frame
		VecN impulses = jacobian.Transpose() * cachedLambda;
		ApplyImpulses(impulses, bodyA, bodyB);

		// @NOTE: Calculate the baumgarte stabilization
		real32 C = Dot(r, r);
		C = Max(0.0f, C - 0.01f);
		real32 Beta = 0.05f;
		baumgarte = (Beta / dt_sec) * C;
	}

	void DistanceConstraint::Solve()
	{
		MatMN JacobianTranspose = jacobian.Transpose();

		// @NOTE: Build the system of equations
		VecN q_dt = GetVelocities(bodyA, bodyB);
		MatMN invMassMatrix = GetInverseMassMatrix(bodyA, bodyB);
		MatMN J_W_Jt = jacobian * invMassMatrix * JacobianTranspose;
		VecN rhs = jacobian * q_dt * -1.0f;
		//rhs[0] -= baumgarte;

		// @NOTE: Solve for the Lagrange multipliers
		VecN lambdaN = GaussSeidel(J_W_Jt, rhs);

		// @NOTE: Apply the impulses
		VecN impulses = JacobianTranspose * lambdaN;
		ApplyImpulses(impulses, bodyA, bodyB);

		// @NOTE: Accumulate the impulses for warm starting
		cachedLambda += lambdaN;
	}

	void DistanceConstraint::PostSolve()
	{
		if (cachedLambda[0] * 0.0f != cachedLambda[0] * 0.0f)
		{
			cachedLambda[0] = 0.0f;
		}

		const float limit = 1e5f;
		if (cachedLambda[0] > limit)
		{
			cachedLambda[0] = limit;
		}

		if (cachedLambda[0] < -limit)
		{
			cachedLambda[0] = -limit;
		}
	}

	inline void GetOrtho(const Vec3f& in, Vec3f* u, Vec3f* v)
	{
		Vec3f n = Normalize(in);
		Vec3f w = (n.z * n.z > 0.9f * 0.9f) ? Vec3f(1, 0, 0) : Vec3f(0, 0, 1);

		*u = Normalize(Cross(w, n));
		*v = Normalize(Cross(n, *u));
		*u = Normalize(Cross(*v, n));
	}

	void PenetrationConstraint::PreSolve(const float dt_sec)
	{
		Vec3f worldAnchorA = bodyA->BodySpaceToWorldSpace(anchorA);
		Vec3f worldAnchorB = bodyB->BodySpaceToWorldSpace(anchorB);

		Vec3f ra = worldAnchorA - bodyA->GetCenterOfMassWorldSpace();
		Vec3f rb = worldAnchorB - bodyB->GetCenterOfMassWorldSpace();
		Vec3f a = worldAnchorA;
		Vec3f b = worldAnchorB;

		const real32 frictionA = bodyA->friction;
		const real32 frictionB = bodyB->friction;
		friction = frictionA * frictionB;


		Vec3f u;
		Vec3f v;
		GetOrtho(normalA, &u, &v);

		// Convert tangent space from model space to world space
		Vec3f normal = RotatePointLHS(bodyA->orientation, normalA);
		u = RotatePointLHS(bodyA->orientation, u);
		v = RotatePointLHS(bodyA->orientation, v);

		//
		//	Penetration Constraint
		//
		jacobian.Zero();

		// First row is the primary distance constraint that holds the anchor points together
		Vec3f J1 = normal * -1.0f;
		jacobian.rows[0][0] = J1.x;
		jacobian.rows[0][1] = J1.y;
		jacobian.rows[0][2] = J1.z;

		Vec3f J2 = Cross(ra, normal * -1.0f);
		jacobian.rows[0][3] = J2.x;
		jacobian.rows[0][4] = J2.y;
		jacobian.rows[0][5] = J2.z;

		Vec3f J3 = normal * 1.0f;
		jacobian.rows[0][6] = J3.x;
		jacobian.rows[0][7] = J3.y;
		jacobian.rows[0][8] = J3.z;

		Vec3f J4 = Cross(rb, normal * 1.0f);
		jacobian.rows[0][9] = J4.x;
		jacobian.rows[0][10] = J4.y;
		jacobian.rows[0][11] = J4.z;

		//
		//	Friction Jacobians
		//
		if (friction > 0.0f)
		{
			Vec3f J1 = u * -1.0f;
			jacobian.rows[1][0] = J1.x;
			jacobian.rows[1][1] = J1.y;
			jacobian.rows[1][2] = J1.z;

			Vec3f J2 = Cross(ra, u * -1.0f);
			jacobian.rows[1][3] = J2.x;
			jacobian.rows[1][4] = J2.y;
			jacobian.rows[1][5] = J2.z;

			Vec3f J3 = u * 1.0f;
			jacobian.rows[1][6] = J3.x;
			jacobian.rows[1][7] = J3.y;
			jacobian.rows[1][8] = J3.z;

			Vec3f J4 = Cross(rb, u * 1.0f);
			jacobian.rows[1][9] = J4.x;
			jacobian.rows[1][10] = J4.y;
			jacobian.rows[1][11] = J4.z;
		}
		if (friction > 0.0f)
		{
			Vec3f J1 = v * -1.0f;
			jacobian.rows[2][0] = J1.x;
			jacobian.rows[2][1] = J1.y;
			jacobian.rows[2][2] = J1.z;

			Vec3f J2 = Cross(ra, v * -1.0f);
			jacobian.rows[2][3] = J2.x;
			jacobian.rows[2][4] = J2.y;
			jacobian.rows[2][5] = J2.z;

			Vec3f J3 = v * 1.0f;
			jacobian.rows[2][6] = J3.x;
			jacobian.rows[2][7] = J3.y;
			jacobian.rows[2][8] = J3.z;

			Vec3f J4 = Cross(rb, v * 1.0f);
			jacobian.rows[2][9] = J4.x;
			jacobian.rows[2][10] = J4.y;
			jacobian.rows[2][11] = J4.z;
		}

		//
		// Apply warm starting from last frame
		//
		VecN impulses = jacobian.Transpose() * cachedLambda;
		ApplyImpulses(impulses, bodyA, bodyB);

		//
		//	Calculate the baumgarte stabilization
		//
		real32 C = Dot((b - a), normal);
		C = std::min(0.0f, C + 0.02f);	// Add slop
		real32 Beta = 0.25f;
		baumgarte = Beta * C / dt_sec;
	}

	void PenetrationConstraint::Solve()
	{
		MatMN JacobianTranspose = jacobian.Transpose();

		// Build the system of equations
		VecN q_dt = GetVelocities(bodyA, bodyB);
		MatMN invMassMatrix = GetInverseMassMatrix(bodyA, bodyB);
		MatMN J_W_Jt = jacobian * invMassMatrix * JacobianTranspose;
		VecN rhs = jacobian * q_dt * -1.0f;
		rhs[0] -= baumgarte;

		// Solve for the Lagrange multipliers
		VecN lambdaN = GaussSeidel(J_W_Jt, rhs);

		// Accumulate the impulses and clamp to within the constraint limits
		VecN oldLambda = cachedLambda;
		cachedLambda += lambdaN;

		const real32 lambdaLimit = 0.0f;
		if (cachedLambda[0] < lambdaLimit)
		{
			cachedLambda[0] = lambdaLimit;
		}

		if (friction > 0.0f)
		{
			real32 umg = friction * 10.0f * 1.0f / (bodyA->invMass + bodyB->invMass);
			real32 normalForce = fabsf(lambdaN[0] * friction);
			real32 maxForce = (umg > normalForce) ? umg : normalForce;

			if (cachedLambda[1] > maxForce)
			{
				cachedLambda[1] = maxForce;
			}
			if (cachedLambda[1] < -maxForce)
			{
				cachedLambda[1] = -maxForce;
			}

			if (cachedLambda[2] > maxForce)
			{
				cachedLambda[2] = maxForce;
			}
			if (cachedLambda[2] < -maxForce)
			{
				cachedLambda[2] = -maxForce;
			}
		}

		lambdaN = cachedLambda - oldLambda;

		// Apply the impulses
		const VecN impulses = JacobianTranspose * lambdaN;
		ApplyImpulses(impulses, bodyA, bodyB);
	}

	void PenetrationConstraint::PostSolve()
	{

	}
}