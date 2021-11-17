#pragma once
#include "core/SolarCore.h"
#include "core/SolarPlatform.h"
#include "SolarBody.h"
namespace cm
{
	struct RigidBodyComponent
	{
		bool32 active;
		real32 invMass;
		real32 elasticity; // @NOTE: COR
		real32 friction;
		Vec3f linearVelocity;
		Vec3f angularVelocity;

		Vec3f forces;// @TODO: Remove, we are impulsed based
		Vec3f torque;// @TODO: Remove, we are impulsed based

		int32 index; // @NOTE: The index to the actual rigidbody in the simulation
	};

	class PhysicsSimulator
	{

	public:
		real32 dt;
		int32 count;
		RigidBody bodies[10000];



		void Begin(real32 dt);
		void AddRigidBody(const Transform& tr, RigidBodyComponent* ri);
		void UpdateEntity(Transform* tr, RigidBodyComponent* ri);

		void Update();
	};

	//struct RigidBody
	//{
	//	Transform trasnform;
	//	CollisionComponent collision;
	//	RigidBodyComponent rigidBody;
	//};

	struct Entity;
	class Input;
	class GameState;
	struct TransientState;
	class CollisionComponent;
	struct Manifold;
	void UpdatePhsyics(GameState* gs, TransientState* ts, Input* input);
	//void TestSignedVolumeProjection();
	//bool GJKIntersection(Entity* bodyA, Entity* bodyB);
	//void GJKClosestPoints(Entity* bodyA, Entity* bodyB, Vec3f* ptOnA, Vec3f* ptOnB);
	//bool GJKManifold(Entity* bodyA, Entity* bodyB, Vec3f* ptOnA, Vec3f* ptOnB, real32 bias);
}

