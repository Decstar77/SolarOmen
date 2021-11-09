#include "SolarBody.h"

namespace cm
{
	Shape::Shape()
	{
		radius = 0.5f;
	}

	Mat3f Shape::GetInertiaTensor() const
	{
		return interiaTensor;
	}

	Vec3f Shape::GetCenterOfMass() const
	{
		return centerOfMass;
	}

	Vec3f Shape::GetSupportPoint(const Vec3f& dir, const Vec3f& pos, const Quatf& orient, const real32 bias) const
	{
		Vec3f resultingPoint = Vec3f(0);
		switch (type)
		{
		case ShapeType::SPHERE:
		{
			resultingPoint = (pos + dir * (radius + bias));
		}break;

		case ShapeType::CUBE:
		case ShapeType::POLYHEDRON:
		{
#if RHS
			Vec3 maxPt = RotatePointRHS(orient, points[0]) + pos;
#else
			Vec3 maxPt = RotatePointLHS(orient, points[0]) + pos;
#endif
			real32 maxDist = Dot(dir, maxPt);
			for (int32 i = 1; i < count; i++) {
#if RHS
				Vec3f pt = RotatePointRHS(orient, points[i]) + pos;
#else
				Vec3f pt = RotatePointLHS(orient, points[i]) + pos;
#endif
				real32 dist = Dot(dir, pt);

				if (dist > maxDist) {
					maxDist = dist;
					maxPt = pt;
				}
			}

			resultingPoint = maxPt + (Normalize(dir) * bias);
		}break;
		}

		return resultingPoint;
	}

	void Shape::CalculateInertiaTensor()
	{
		switch (type)
		{
		case ShapeType::SPHERE:
		{
			centerOfMass = Vec3f(0.0);
			interiaTensor = CalcShpereInertiaTensor(radius);
		}break;
		case ShapeType::CUBE:
		{
			Vec3f max = points[0];
			Vec3f min = points[0];

			for (int32 i = 0; i < count; i++)
			{
				centerOfMass += points[i];

				max = Max(max, points[i]);
				min = Max(min, points[i]);
			}
			centerOfMass = centerOfMass / (real32)count;

			CalcBoxInertiaTensor(
				max.x - min.x,
				max.y - min.y,
				max.z - min.z,
				1.0f, centerOfMass);
		}break;
		case ShapeType::POLYHEDRON:
		{
		}break;
		}
	}

	bool Shape::CollisionCheck(Vec3f aP, Shape* shapeA, Vec3f bP, Shape* shapeB, Contact* info)
	{
		Vec3f ab = bP - aP;
		real32 r = shapeA->radius + shapeB->radius;
		if (MagSqrd(ab) <= (r * r))
		{
			info->normal = Normalize(ab);

			//info->ptOnA_LocalSpace = info->normal * shapeA->radius;
			//info->ptOnB_LocalSpace = -1.0f * info->normal * shapeB->radius;

			info->ptOnA_WorldSpace = aP + info->normal * shapeA->radius;
			info->ptOnB_WorldSpace = bP - info->normal * shapeB->radius;

			return true;
		}

		return false;
	}


	Mat3f Shape::CalcBoxInertiaTensor(real32 w, real32 h, real32 d, real32 m, Vec3f cm)
	{
		Mat3f tensor = Mat3f(0);
		tensor[0][0] = (h * h + d * d) / 12.0f;
		tensor[1][1] = (w * w + d * d) / 12.0f;
		tensor[2][2] = (w * w + h * h) / 12.0f;

		Vec3f R = Vec3f(0, 0, 0) - cm;
		real32 R2 = MagSqrd(R);

		Mat3f patTensor = Mat3f(0);
		patTensor[0] = Vec3f(R2 - R.x * R.x, R.x * R.y, R.x * R.z);
		patTensor[1] = Vec3f(R.y * R.x, R2 - R.y * R.y, R.y * R.z);
		patTensor[2] = Vec3f(R.z * R.x, R.z * R.y, R2 - R.z * R.z);

		tensor = m * (tensor + patTensor);
		return tensor;
	}

	Mat3f Shape::CalcShpereInertiaTensor(real32 r)
	{
		Mat3f t = Mat3f(1);
		t[0][0] = 2.0f * r * r / 5.0f;
		t[1][1] = 2.0f * r * r / 5.0f;
		t[2][2] = 2.0f * r * r / 5.0f;

		return t;
	}

	RigidBody::RigidBody() :
		position(0.0f),
		orientation(0.0f, 0.0f, 0.0f, 1.0f), invMass(0), elasticity(0.5f), friction(0.5f)
	{
		linearVelocity = Vec3f(0);
		shape = Shape();
		shape.radius = 0.5f;
	}


	Vec3f RigidBody::GetCenterOfMassWorldSpace() const {
		Vec3f centerOfMass = shape.GetCenterOfMass();
#if RHS
		Vec3f pos = position + RotatePointRHS(orientation, centerOfMass);
#else
		Vec3f pos = position + RotatePointLHS(orientation, centerOfMass);
#endif

		return pos;
	}

	Vec3f RigidBody::GetCenterOfMassModelSpace() const {
		const Vec3f centerOfMass = shape.GetCenterOfMass();

		return centerOfMass;
	}


	Vec3f RigidBody::WorldSpaceToBodySpace(const Vec3f& worldPt) const {
		Vec3f tmp = worldPt - GetCenterOfMassWorldSpace();
		Quatf inverseOrient = Conjugate(Normalize(orientation));
#if RHS
		Vec3f bodySpace = RotatePointRHS(inverseOrient, tmp);
#else
		Vec3f bodySpace = RotatePointLHS(inverseOrient, tmp);
#endif

		return bodySpace;
	}


	Vec3f RigidBody::BodySpaceToWorldSpace(const Vec3f& worldPt) const {
#if RHS
		Vec3f worldSpace = GetCenterOfMassWorldSpace() + RotatePointRHS(orientation, worldPt);
#else
		Vec3f worldSpace = GetCenterOfMassWorldSpace() + RotatePointLHS(orientation, worldPt);
#endif
		return worldSpace;
	}


	Mat3f RigidBody::GetInverseInertiaTensorBodySpace() const {
		Mat3f inertiaTensor = shape.GetInertiaTensor();
		Mat3f invInertiaTensor = invMass * Inverse(inertiaTensor);

		return invInertiaTensor;
	}


	Mat3f RigidBody::GetInverseInertiaTensorWorldSpace() const {
		Mat3f inertiaTensor = shape.GetInertiaTensor();
		Mat3f invInertiaTensor = invMass * Inverse(inertiaTensor);
		Mat3f orient = QuatToMat3(orientation);
#if RHS
		invInertiaTensor = ChangeOfBasisRHS(orient, invInertiaTensor);
#else
		invInertiaTensor = ChangeOfBasisLHS(orient, invInertiaTensor);
#endif

		return invInertiaTensor;
	}


	void RigidBody::ApplyImpulse(const Vec3f& impulsePoint, const Vec3f& impulse) {
		if (0.0f == invMass) {
			return;
		}

		// impulsePoint is the world space location of the application of the impulse
		// impulse is the world space direction and magnitude of the impulse
		ApplyImpulseLinear(impulse);

		Vec3f position = GetCenterOfMassWorldSpace();	// applying impulses must produce torques through the center of mass
		Vec3f r = impulsePoint - position;
		Vec3f dL = Cross(r, impulse);					// this is in world space
		ApplyImpulseAngular(dL);
	}


	void RigidBody::ApplyImpulseLinear(const Vec3f& impulse) {
		if (0.0f == invMass) {
			return;
		}

		// p = mv
		// dp = m dv = J
		// => dv = J / m
		linearVelocity += impulse * invMass;
	}


	void RigidBody::ApplyImpulseAngular(const Vec3f& impulse) {
		if (0.0f == invMass) {
			return;
		}

		// L = I w = r x p
		// dL = I dw = r x J 
		// => dw = I^-1 * ( r x J )
		angularVelocity += impulse * GetInverseInertiaTensorWorldSpace();

		const float maxAngularSpeed = 30.0f; // 30 rad/s is fast enough for us. But feel free to adjust.
		if (MagSqrd(angularVelocity) > maxAngularSpeed * maxAngularSpeed) {
			angularVelocity = Normalize(angularVelocity);
			angularVelocity = angularVelocity * maxAngularSpeed;
		}
	}


	void RigidBody::Update(real32 dt_sec) {
		position += linearVelocity * dt_sec;

		// okay, we have an angular velocity around the center of mass, this needs to be
		// converted somehow to relative to model position.  This way we can properly update
		// the orientation of the model.
		Vec3f positionCM = GetCenterOfMassWorldSpace();
		Vec3f cmToPos = position - positionCM;

		// Total Torque is equal to external applied torques + internal torque (precession)
		// T = T_external + omega x I * omega
		// T_external = 0 because it was applied in the collision response function
		// T = Ia = w x I * w
		// a = I^-1 ( w x I * w )
		Mat3f orientation = QuatToMat3(this->orientation);
#if RHS
		Mat3f inertiaTensor = ChangeOfBasisRHS(orientation, shape.GetInertiaTensor());
#else
		Mat3f inertiaTensor = ChangeOfBasisLHS(orientation, shape.GetInertiaTensor());
#endif

		Vec3f alpha = (Cross(angularVelocity, angularVelocity * inertiaTensor)) * Inverse(inertiaTensor);
		angularVelocity += alpha * dt_sec;

		// Update orientation
		Vec3f dAngle = angularVelocity * dt_sec;
		Quatf dq = QuatFromAxisAngle(dAngle, Mag(dAngle));
		this->orientation = Normalize(dq * this->orientation);

#if RHS
		position = positionCM + RotatePointRHS(dq, cmToPos);
#else
		position = positionCM + RotatePointLHS(dq, cmToPos);
#endif
		// Now get the new model position
	}

	void Contact::Resolve()
	{
#if 0
		real32 invMassA = bodyA->invMass;
		real32 invMassB = bodyB->invMass;

		real32 elasticityA = bodyA->elasticity;
		real32 elasticityB = bodyB->elasticity;
		real32 elasticity = elasticityA * elasticityB;

		Vec3f n = normal;
		Vec3f vab = bodyA->linearVelocity - bodyB->linearVelocity;
		real32 j = -(1.0f + elasticity) * Dot(vab, n) / (invMassA + invMassB);
		Vec3f vJ = n * j;

		bodyA->ApplyImpulseLinear(1.0f * vJ);
		bodyB->ApplyImpulseLinear(-1.0f * vJ);

		// @NOTE: Resolve interpentration
		real32 tA = bodyA->invMass / (bodyA->invMass + bodyB->invMass);
		real32 tB = bodyB->invMass / (bodyA->invMass + bodyB->invMass);

		Vec3f ds = ptOnB_WorldSpace - ptOnA_WorldSpace;
		bodyA->position += ds * tA;
		bodyB->position -= ds * tB;
#else
		const Vec3f ptOnA = ptOnA_WorldSpace;
		const Vec3f ptOnB = ptOnB_WorldSpace;

		const real32 elasticityA = bodyA->elasticity;
		const real32 elasticityB = bodyB->elasticity;
		const real32 elasticity = elasticityA * elasticityB;

		const real32 invMassA = bodyA->invMass;
		const real32 invMassB = bodyB->invMass;

		const Mat3f invWorldInertiaA = bodyA->GetInverseInertiaTensorWorldSpace();
		const Mat3f invWorldInertiaB = bodyB->GetInverseInertiaTensorWorldSpace();

		const Vec3f n = normal;

		const Vec3f ra = ptOnA - bodyA->GetCenterOfMassWorldSpace();
		const Vec3f rb = ptOnB - bodyB->GetCenterOfMassWorldSpace();

		const Vec3f angularJA = Cross((Cross(ra, n) * invWorldInertiaA), ra);
		const Vec3f angularJB = Cross((Cross(rb, n) * invWorldInertiaB), rb);
		const real32 angularFactor = Dot(angularJA + angularJB, n);

		// Get the world space velocity of the motion and rotation
		const Vec3f velA = bodyA->linearVelocity + Cross(bodyA->angularVelocity, ra);
		const Vec3f velB = bodyB->linearVelocity + Cross(bodyB->angularVelocity, rb);

		// Calculate the collision impulse
		const Vec3f vab = velA - velB;
		const real32 ImpulseJ = (1.0f + elasticity) * Dot(vab, n) / (invMassA + invMassB + angularFactor);
		const Vec3f vectorImpulseJ = n * ImpulseJ;

		bodyA->ApplyImpulse(ptOnA, vectorImpulseJ * -1.0f);
		bodyB->ApplyImpulse(ptOnB, vectorImpulseJ * 1.0f);

		//
		// Calculate the impulse caused by friction
		//

		const real32 frictionA = bodyA->friction;
		const real32 frictionB = bodyB->friction;
		const real32 friction = frictionA * frictionB;

		// Find the normal direction of the velocity with respect to the normal of the collision
		const Vec3f velNorm = n * Dot(n, vab);

		// Find the tangent direction of the velocity with respect to the normal of the collision
		const Vec3f velTang = vab - velNorm;

		// Get the tangential velocities relative to the other body
		Vec3f relativeVelTang = Normalize(velTang);

		const Vec3f inertiaA = Cross((Cross(ra, relativeVelTang) * invWorldInertiaA), ra);
		const Vec3f inertiaB = Cross((Cross(rb, relativeVelTang) * invWorldInertiaB), rb);
		const real32 invInertia = Dot(inertiaA + inertiaB, relativeVelTang);

		// Calculate the tangential impulse for friction
		const real32 reducedMass = 1.0f / (bodyA->invMass + bodyB->invMass + invInertia);
		const Vec3f impulseFriction = velTang * reducedMass * friction;

		// Apply kinetic friction
		bodyA->ApplyImpulse(ptOnA, impulseFriction * -1.0f);
		bodyB->ApplyImpulse(ptOnB, impulseFriction * 1.0f);

		// @NOTE: Resolve interpentration
		real32 tA = bodyA->invMass / (bodyA->invMass + bodyB->invMass);
		real32 tB = bodyB->invMass / (bodyA->invMass + bodyB->invMass);

		Vec3f ds = ptOnB - ptOnA;
		bodyA->position += ds * tA;
		bodyB->position -= ds * tB;
#endif
	}

}