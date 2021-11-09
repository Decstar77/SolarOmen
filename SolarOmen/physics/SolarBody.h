#pragma once

#include "../core/SolarCore.h"
#define RHS 0

namespace cm
{
	enum class ShapeType
	{
		SPHERE,
		CUBE,
		POLYHEDRON,
	};

	class Shape
	{
	public:
		ShapeType type;
		real32 radius;
		int32 count;
		Vec3f points[16];
		Vec3f centerOfMass;
		Mat3f interiaTensor;	// @SPEED: Cache these, do we even need the non inverse ?
		Mat3f invInertiaTensor;	// @SPEED: Cache these

	public:
		Shape();
		Mat3f GetInertiaTensor() const;
		Vec3f GetCenterOfMass() const;
		// @SPEED: Cache these in GJK ?
		Vec3f GetSupportPoint(const Vec3f& dir, const Vec3f& pos, const Quatf& orient, const real32 bias) const;

		void CalculateInertiaTensor();
		static bool CollisionCheck(Vec3f aP, Shape* shapeA, Vec3f bP, Shape* shapeB, class Contact* info);
	private:
		Mat3f CalcBoxInertiaTensor(real32 w, real32 h, real32 d, real32 m, Vec3f cm);
		Mat3f CalcShpereInertiaTensor(real32 r);
	};

	class RigidBody
	{
	public:
		Vec3f position;
		Quatf orientation;
		Vec3f scale;

		Vec3f forces;
		Vec3f torque;

		Vec3f linearVelocity;
		Vec3f angularVelocity;

		real32 invMass;
		real32 elasticity; // @NOTE: COR
		real32 friction;

		Shape shape;

		Vec3f GetCenterOfMassWorldSpace() const;
		Vec3f GetCenterOfMassModelSpace() const;

		Vec3f WorldSpaceToBodySpace(const Vec3f& pt) const;
		Vec3f BodySpaceToWorldSpace(const Vec3f& pt) const;

		Mat3f GetInverseInertiaTensorBodySpace() const;
		Mat3f GetInverseInertiaTensorWorldSpace() const;

		void ApplyImpulse(const Vec3f& impulsePoint, const Vec3f& impulse);
		void ApplyImpulseLinear(const Vec3f& impulse);
		void ApplyImpulseAngular(const Vec3f& impulse);

		void Update(real32 dt_sec);

		RigidBody();
	};

	class Contact
	{
	public:
		Vec3f ptOnA_WorldSpace;
		Vec3f ptOnB_WorldSpace;
		Vec3f normal;

		RigidBody* bodyA;
		RigidBody* bodyB;
		void Resolve();
	};

	class CollisionManifold
	{
	public:
		int32 count;
		Contact contacts[4];

		RigidBody* bodyA;
		RigidBody* bodyB;
	};
}
