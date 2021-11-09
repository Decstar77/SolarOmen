#pragma once
#include "../../core/SolarCore.h"

namespace cm::MK2
{
	enum class PhysicsShapeType
	{
		SHPERE,
		BOX,
	};

	class PhysicsShape
	{
	public:
		PhysicsShapeType type;

	public:
		Vec3f GetSupport(const Vec3f& dir, const Vec3f& pos, const Quatf& orient, const float bias) const;
		real32 GetFastestLinearSpeed(const Vec3f& angularVelocity, const Vec3f& dir) const;

		Mat3f GetInertiaTensor() const;
		Vec3f GetCenterOfMass() const;

		real32 radius;
		Vec3f centerOfMass;
		Vec3f points[8];

		static PhysicsShape CreateBox();

	private:
	};

	class RigidBody
	{
	public:
		RigidBody();

		Vec3f position;
		Quatf orientation;
		Vec3f linearVelocity;
		Vec3f angularVelocity;

		real32 invMass;
		real32 elasticity;
		real32 friction;

		PhysicsShape shape;

		Vec3f GetCenterOfMassWorldSpace() const;
		Vec3f GetCenterOfMassModelSpace() const;

		Vec3f WorldSpaceToBodySpace(const Vec3f& pt) const;
		Vec3f BodySpaceToWorldSpace(const Vec3f& pt) const;

		Mat3f GetInverseInertiaTensorBodySpace() const;
		Mat3f GetInverseInertiaTensorWorldSpace() const;

		void ApplyImpulse(const Vec3f& impulsePoint, const Vec3f& impulse);
		void ApplyImpulseLinear(const Vec3f& impulse);
		void ApplyImpulseAngular(const Vec3f& impulse);

		void Update(const float dt_sec);
	};
}
