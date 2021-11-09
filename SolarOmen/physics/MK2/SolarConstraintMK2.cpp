#include "SolarConstraintMK2.h"

namespace cm::MK2
{
	MatMN Constraint::GetInverseMassMatrix() const
	{
		MatMN invMassMatrix(12, 12);
		invMassMatrix.Zero();

		invMassMatrix.rows[0][0] = bodyA->invMass;
		invMassMatrix.rows[1][1] = bodyA->invMass;
		invMassMatrix.rows[2][2] = bodyA->invMass;

		Mat3f invInertiaA = bodyA->GetInverseInertiaTensorWorldSpace();
		for (int i = 0; i < 3; i++) {
			invMassMatrix.rows[3 + i][3 + 0] = invInertiaA[i][0];
			invMassMatrix.rows[3 + i][3 + 1] = invInertiaA[i][1];
			invMassMatrix.rows[3 + i][3 + 2] = invInertiaA[i][2];
		}

		invMassMatrix.rows[6][6] = bodyB->invMass;
		invMassMatrix.rows[7][7] = bodyB->invMass;
		invMassMatrix.rows[8][8] = bodyB->invMass;

		Mat3f invInertiaB = bodyB->GetInverseInertiaTensorWorldSpace();
		for (int i = 0; i < 3; i++) {
			invMassMatrix.rows[9 + i][9 + 0] = invInertiaB[i][0];
			invMassMatrix.rows[9 + i][9 + 1] = invInertiaB[i][1];
			invMassMatrix.rows[9 + i][9 + 2] = invInertiaB[i][2];
		}

		return invMassMatrix;
	}

	VecN Constraint::GetVelocities() const
	{
		VecN q_dt(12);

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

	void Constraint::ApplyImpulses(const VecN& impulses)
	{
		Vec3f forceInternalA(0.0f);
		Vec3f torqueInternalA(0.0f);
		Vec3f forceInternalB(0.0f);
		Vec3f torqueInternalB(0.0f);

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

	void ConstraintDistance::PreSolve(const real32 dt_sec)
	{
		// Get the world space position of the hinge from A's orientation
		const Vec3f worldAnchorA = bodyA->BodySpaceToWorldSpace(anchorA);

		// Get the world space position of the hinge from B's orientation
		const Vec3f worldAnchorB = bodyB->BodySpaceToWorldSpace(anchorB);

		const Vec3f r = worldAnchorB - worldAnchorA;
		const Vec3f ra = worldAnchorA - bodyA->GetCenterOfMassWorldSpace();
		const Vec3f rb = worldAnchorB - bodyB->GetCenterOfMassWorldSpace();
		const Vec3f a = worldAnchorA;
		const Vec3f b = worldAnchorB;

		m_Jacobian.Zero();

		Vec3f J1 = (a - b) * 2.0f;
		m_Jacobian.rows[0][0] = J1.x;
		m_Jacobian.rows[0][1] = J1.y;
		m_Jacobian.rows[0][2] = J1.z;

		Vec3f J2 = Cross(ra, (a - b) * 2.0f);
		m_Jacobian.rows[0][3] = J2.x;
		m_Jacobian.rows[0][4] = J2.y;
		m_Jacobian.rows[0][5] = J2.z;

		Vec3f J3 = (b - a) * 2.0f;
		m_Jacobian.rows[0][6] = J3.x;
		m_Jacobian.rows[0][7] = J3.y;
		m_Jacobian.rows[0][8] = J3.z;

		Vec3f J4 = Cross(rb, (b - a) * 2.0f);
		m_Jacobian.rows[0][9] = J4.x;
		m_Jacobian.rows[0][10] = J4.y;
		m_Jacobian.rows[0][11] = J4.z;

		const VecN impulses = m_Jacobian.Transpose() * m_cachedLambda;
		ApplyImpulses(impulses);
	}

	static VecN LCP_GaussSeidel(const MatN& A, const VecN& b) {
		const int N = b.N;
		VecN x(N);
		x.Zero();

		for (int iter = 0; iter < N; iter++) {
			for (int i = 0; i < N; i++) {
				float dx = (b[i] - A.rows[i].Dot(x)) / A.rows[i][i];
				if (dx * 0.0f == dx * 0.0f) {
					x[i] = x[i] + dx;
				}
			}
		}
		return x;
	}

	void ConstraintDistance::Solve()
	{
		const MatMN JacobianTranspose = m_Jacobian.Transpose();

		// Build the system of equations
		const VecN q_dt = GetVelocities();
		const MatMN invMassMatrix = GetInverseMassMatrix();
		const MatMN J_W_Jt = m_Jacobian * invMassMatrix * JacobianTranspose;
		VecN rhs = m_Jacobian * q_dt * -1.0f;
		//rhs[0] -= m_baumgarte;

		// Solve for the Lagrange multipliers
		const VecN lambdaN = LCP_GaussSeidel(J_W_Jt, rhs);

		// Apply the impulses
		const VecN impulses = JacobianTranspose * lambdaN;
		ApplyImpulses(impulses);

		// Accumulate the impulses for warm starting
		m_cachedLambda += lambdaN;
	}

	void ConstraintDistance::PostSolve()
	{
		// Limit the warm starting to reasonable limits
		if (m_cachedLambda[0] * 0.0f != m_cachedLambda[0] * 0.0f) {
			m_cachedLambda[0] = 0.0f;
		}
		const float limit = 1e5f;
		if (m_cachedLambda[0] > limit) {
			m_cachedLambda[0] = limit;
		}
		if (m_cachedLambda[0] < -limit) {
			m_cachedLambda[0] = -limit;
		}
	}
}