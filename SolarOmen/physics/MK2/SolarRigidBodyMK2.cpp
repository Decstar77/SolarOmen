#include "SolarRigidBodyMK2.h"


namespace cm::MK2
{

	/*
	====================================================
	RigidBody::RigidBody
	====================================================
	*/
	RigidBody::RigidBody() :
		position(0.0f),
		orientation(0.0f, 0.0f, 0.0f, 1.0f), invMass(0), elasticity(0.5f), friction(0.5f)
	{
		linearVelocity = Vec3f(0);
	}

	/*
	====================================================
	RigidBody::GetCenterOfMassWorldSpace
	====================================================
	*/
	Vec3f RigidBody::GetCenterOfMassWorldSpace() const {
		Vec3f centerOfMass = shape.GetCenterOfMass();
		Vec3f pos = position + RotatePointRHS(orientation, centerOfMass);

		return pos;
	}

	/*
	====================================================
	RigidBody::GetCenterOfMassModelSpace
	====================================================
	*/
	Vec3f RigidBody::GetCenterOfMassModelSpace() const {
		const Vec3f centerOfMass = shape.GetCenterOfMass();

		return centerOfMass;
	}

	/*
	====================================================
	RigidBody::WorldSpaceToBodySpace
	====================================================
	*/
	Vec3f RigidBody::WorldSpaceToBodySpace(const Vec3f& worldPt) const {
		Vec3f tmp = worldPt - GetCenterOfMassWorldSpace();
		Quatf inverseOrient = Conjugate(Normalize(orientation));
		Vec3f bodySpace = RotatePointRHS(inverseOrient, tmp);

		return bodySpace;
	}

	/*
	====================================================
	RigidBody::BodySpaceToWorldSpace
	====================================================
	*/
	Vec3f RigidBody::BodySpaceToWorldSpace(const Vec3f& worldPt) const {
		Vec3f worldSpace = GetCenterOfMassWorldSpace() + RotatePointRHS(orientation, worldPt);

		return worldSpace;
	}

	/*
	====================================================
	RigidBody::GetInverseInertiaTensorBodySpace
	====================================================
	*/
	Mat3f RigidBody::GetInverseInertiaTensorBodySpace() const {
		Mat3f inertiaTensor = shape.GetInertiaTensor();
		Mat3f invInertiaTensor = invMass * Inverse(inertiaTensor);

		return invInertiaTensor;
	}

	/*
	====================================================
	RigidBody::GetInverseInertiaTensorWorldSpace
	====================================================
	*/
	Mat3f RigidBody::GetInverseInertiaTensorWorldSpace() const {
		Mat3f inertiaTensor = shape.GetInertiaTensor();
		Mat3f invInertiaTensor = invMass * Inverse(inertiaTensor);
		Mat3f orient = QuatToMat3(orientation);

		invInertiaTensor = Transpose(orient) * invInertiaTensor * (orient);

		return invInertiaTensor;
	}

	/*
	====================================================
	RigidBody::ApplyImpulse
	====================================================
	*/
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

	/*
	====================================================
	RigidBody::ApplyImpulseLinear
	====================================================
	*/
	void RigidBody::ApplyImpulseLinear(const Vec3f& impulse) {
		if (0.0f == invMass) {
			return;
		}

		// p = mv
		// dp = m dv = J
		// => dv = J / m
		linearVelocity += impulse * invMass;
	}

	/*
	====================================================
	RigidBody::ApplyImpulseAngular
	====================================================
	*/
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

	/*
	====================================================
	RigidBody::Update
	====================================================
	*/
	void RigidBody::Update(const float dt_sec) {
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
		Mat3f inertiaTensor = Transpose(orientation) * shape.GetInertiaTensor() * orientation;
		Vec3f alpha = (Cross(angularVelocity, angularVelocity * inertiaTensor)) * Inverse(inertiaTensor);
		angularVelocity += alpha * dt_sec;

		// Update orientation
		Vec3f dAngle = angularVelocity * dt_sec;
		Quatf dq = QuatFromAxisAngle(dAngle, Mag(dAngle));
		this->orientation = Normalize(this->orientation * dq);

		// Now get the new model position
		position = positionCM + RotatePointRHS(dq, cmToPos);
	}


	Vec3f PhysicsShape::GetSupport(const Vec3f& dir, const Vec3f& pos, const Quatf& orient, const float bias) const
	{
		switch (type)
		{
		case PhysicsShapeType::BOX:
		{
			// Find the point in furthest in direction
			Vec3f maxPt = RotatePointRHS(orient, points[0]) + pos;
			real32 maxDist = Dot(dir, maxPt);
			for (int32 i = 1; i < 8; i++)
			{
				Vec3f pt = RotatePointRHS(orient, points[i]) + pos;
				real32 dist = Dot(dir, pt);

				if (dist - maxDist > 0.0001f)
				{
					maxDist = dist;
					maxPt = pt;
				}
			}

			Vec3f norm = Normalize(dir) * bias;

			return maxPt + norm;
		}break;
		}

		return Vec3f(0);
	}

	real32 PhysicsShape::GetFastestLinearSpeed(const Vec3f& angularVelocity, const Vec3f& dir) const
	{
		switch (type)
		{
		case PhysicsShapeType::BOX:
		{
			real32 maxSpeed = 0.0f;
			for (int32 i = 0; i < 8; i++)
			{
				Vec3f r = points[i] - centerOfMass;
				Vec3f linearVelocity = Cross(angularVelocity, r);
				real32 speed = Dot(dir, linearVelocity);
				if (speed > maxSpeed)
				{
					maxSpeed = speed;
				}
			}

			return maxSpeed;
		}break;
		}

		return 0.0f;
	}

	Mat3f PhysicsShape::GetInertiaTensor() const
	{
		switch (type)
		{
		case PhysicsShapeType::SHPERE:
		{
			Mat3f tensor = Mat3f(0);
			tensor.row0[0] = 2.0f * radius * radius / 5.0f;
			tensor.row1[1] = 2.0f * radius * radius / 5.0f;
			tensor.row2[2] = 2.0f * radius * radius / 5.0f;

			return tensor;
		}break;

		case PhysicsShapeType::BOX:
		{
			// @HACK: Only works for box with width = height = depth = 1.0f
			real32 dx = 1.0f;
			real32 dy = 1.0f;
			real32 dz = 1.0f;

			Mat3f tensor = Mat3f(0);
			tensor.row0[0] = (dy * dy + dz * dz) / 12.0f;
			tensor.row1[1] = (dx * dx + dz * dz) / 12.0f;
			tensor.row2[2] = (dx * dx + dy * dy) / 12.0f;

			//// Now we need to use the parallel axis theorem to get the inertia tensor for a box
			//// that is not centered around the origin
			//Vec3 cm;
			//cm.x = (m_bounds.maxs.x + m_bounds.mins.x) * 0.5f;
			//cm.y = (m_bounds.maxs.y + m_bounds.mins.y) * 0.5f;
			//cm.z = (m_bounds.maxs.z + m_bounds.mins.z) * 0.5f;
			//
			//const Vec3 R = Vec3(0, 0, 0) - cm;	// the displacement from center of mass to the origin
			//const float R2 = R.GetLengthSqr();
			//Mat3 patTensor;
			//patTensor.rows[0] = Vec3(R2 - R.x * R.x, R.x * R.y, R.x * R.z);
			//patTensor.rows[1] = Vec3(R.y * R.x, R2 - R.y * R.y, R.y * R.z);
			//patTensor.rows[2] = Vec3(R.z * R.x, R.z * R.y, R2 - R.z * R.z);
			//
			//// Now we need to add the center of mass tensor and the parallel axis theorem tensor together;
			//tensor += patTensor;
			return tensor;
		}break;
		}

		return Mat3f(0);
	}

	Vec3f PhysicsShape::GetCenterOfMass() const
	{
		return centerOfMass;
	}

	PhysicsShape PhysicsShape::CreateBox()
	{
		real32 t = 0.5f;
		PhysicsShape box = {};
		box.type = PhysicsShapeType::BOX;

		box.points[0] = Vec3f(-t, -t, -t);
		box.points[1] = Vec3f(t, -t, -t);
		box.points[2] = Vec3f(-t, t, -t);
		box.points[3] = Vec3f(-t, -t, t);
		box.points[4] = Vec3f(t, t, t);
		box.points[5] = Vec3f(-t, t, t);
		box.points[6] = Vec3f(t, -t, t);
		box.points[7] = Vec3f(t, t, -t);

		// @HACK: 
		box.centerOfMass = Vec3f(0);

		return box;
	};


}

