#include "SolarPhysics.h"
#include "SolarGJK.h"
#include "SolarConstraints.h"

#include "../SolarOmen.h"
#include "../Debug.h"
namespace cm
{
	// @TODO: Jus pass the boolemn physics simulator
	void UpdatePhsyics(GameState* gs, TransientState* ts, Input* input)
	{
		PROFILE_FUNCTION();
		PhysicsSimulator* ps = &ts->physicsSimulator;
		int32 numSteps = 2;
		ps->dt = ps->dt / numSteps;
		for (int32 step = 0; step < numSteps; step++)
		{
			ps->Update();
		}
	}

	void PhysicsSimulator::Begin(real32 dt)
	{
		this->dt = dt;
		count = 0;
	}

	void PhysicsSimulator::AddRigidBody(const Transform& tr, RigidBodyComponent* ri)
	{
		RigidBody body = {};
#if RHS
		body.position.x = tr.position.x;
		body.position.y = tr.position.z;
		body.position.z = tr.position.y;

		body.scale = tr.scale;

		body.orientation.x = -tr.orientation.x;
		body.orientation.y = -tr.orientation.z;
		body.orientation.z = -tr.orientation.y;
		body.orientation.w = tr.orientation.w;

		body.linearVelocity.x = ri->linearVelocity.x;
		body.linearVelocity.y = ri->linearVelocity.z;
		body.linearVelocity.z = ri->linearVelocity.y;

		body.angularVelocity.x = ri->angularVelocity.x;
		body.angularVelocity.y = ri->angularVelocity.z;
		body.angularVelocity.z = ri->angularVelocity.y;
#else 
		body.position = tr.position;
		body.orientation = tr.orientation;
		body.scale = tr.scale;
		body.linearVelocity = ri->linearVelocity;
		body.angularVelocity = ri->angularVelocity;
#endif

		body.invMass = ri->invMass;
		body.elasticity = ri->elasticity;
		body.friction = ri->friction;

		int32 index = count;
		count++;

		bodies[index] = body;
		ri->index = index;
	}

	void PhysicsSimulator::UpdateEntity(Transform* tr, RigidBodyComponent* ri)
	{
		RigidBody* body = &bodies[ri->index];
#if RHS
		tr->position.x = body->position.x;
		tr->position.y = body->position.z;
		tr->position.z = body->position.y;

		tr->scale = body->scale;

		tr->orientation.x = -body->orientation.x;
		tr->orientation.y = -body->orientation.z;
		tr->orientation.z = -body->orientation.y;
		tr->orientation.w = body->orientation.w;

		ri->linearVelocity.x = body->linearVelocity.x;
		ri->linearVelocity.y = body->linearVelocity.z;
		ri->linearVelocity.z = body->linearVelocity.y;

		ri->angularVelocity.x = body->angularVelocity.x;
		ri->angularVelocity.y = body->angularVelocity.z;
		ri->angularVelocity.z = body->angularVelocity.y;
#else
		tr->position = body->position;
		tr->orientation = body->orientation;
		tr->scale = body->scale;
		ri->linearVelocity = body->linearVelocity;
		ri->angularVelocity = body->angularVelocity;
#endif

	}

	void PhysicsSimulator::Update()
	{
		for (int32 i = 0; i < count; i++)
		{
			RigidBody* body = &bodies[i];

#if 0
			body->shape.count = 1;
			body->shape.type = ShapeType::SPHERE;
			body->shape.radius = body->scale.x * 0.5f;
#else
			body->shape.type = ShapeType::CUBE;
			real32 t = body->scale.x * 0.5f;
			body->shape.count = 8;
			body->shape.points[0] = Vec3(-t, -t, -t);
			body->shape.points[1] = Vec3(t, -t, -t);
			body->shape.points[2] = Vec3(-t, t, -t);
			body->shape.points[3] = Vec3(t, t, -t);
			body->shape.points[4] = Vec3(-t, -t, t);
			body->shape.points[5] = Vec3(t, -t, t);
			body->shape.points[6] = Vec3(-t, t, t);
			body->shape.points[7] = Vec3(t, t, t);
#endif
			bodies->shape.CalculateInertiaTensor();
		}

		for (int32 i = 0; i < count; i++)
		{
			RigidBody* body = &bodies[i];
#if RHS
			body->ApplyImpulseLinear(Vec3f(0, 0, -10.0f) * dt * (1.0f / body->invMass));
#else
			body->ApplyImpulseLinear(Vec3f(0, -10.0f, 0) * dt * (1.0f / body->invMass));
#endif

		}

		static DistanceConstraint dc;

		static bool once = true;
		if (once)
		{
			dc.bodyA = &bodies[1];
			dc.bodyB = &bodies[0];

			dc.anchorA = dc.bodyA->WorldSpaceToBodySpace(dc.bodyA->position);
			dc.anchorB = dc.bodyB->WorldSpaceToBodySpace(dc.bodyA->position);

			//bodies[0].ApplyImpulse(Vec3f(1.0, 0, 0), Vec3f(0, 0, 1.0));
			//bodies[0].ApplyImpulse(Vec3f(-1.0, 0, 0), Vec3f(0, 0, 1.0));
			//bodies[0].orientation = EulerToQuat(Vec3f(2, 45.0f, 34));
			once = false;
		}

		std::vector<PenetrationConstraint> penetrations;

#if 1
		for (int32 i = 0; i < count; i++)
		{
			RigidBody* bodyA = &bodies[i];

			for (int32 j = i + 1; j < count; j++)
			{
				RigidBody* bodyB = &bodies[j];

				if (bodyA->invMass != 0.0f || bodyB->invMass != 0.0f)
				{
#if 1
					Vec3f pA;
					Vec3f pB;

					Contact info = {};
					if (EPAIntersection(bodyA, bodyB, &pA, &pB))
					{
						//DEBUGDrawPoint(pA);
						//DEBUGDrawPoint(pB);
						info.normal = Normalize(pB - pA);
						info.ptOnA_WorldSpace = pA - info.normal * 0.001f;
						info.ptOnB_WorldSpace = pB - info.normal * 0.001f;
						info.bodyA = bodyA;
						info.bodyB = bodyB;

						PenetrationConstraint pc;
						pc.bodyA = bodyA;
						pc.bodyB = bodyB;
						pc.anchorA = bodyA->WorldSpaceToBodySpace(info.ptOnA_WorldSpace);
						pc.anchorB = bodyB->WorldSpaceToBodySpace(info.ptOnB_WorldSpace);
						pc.normalA = Normalize(RotatePointLHS(Conjugate(bodyA->orientation), info.normal * -1.0f));

						//penetrations.push_back(pc);
						info.Resolve();
					}

#else
					Contact info = {};
					if (Shape::CollisionCheck(
						bodyA->position, &bodyA->shape,
						bodyB->position, &bodyB->shape, &info))
					{
						info.bodyA = bodyA;
						info.bodyB = bodyB;
						info.Resolve();
					}
#endif
				}
			}
		}
#endif

		for (PenetrationConstraint& pc : penetrations)
		{
			pc.PreSolve(dt);
		}

		for (int i = 0; i < 5; i++)
		{
			for (PenetrationConstraint& pc : penetrations)
			{
				pc.Solve();
			}
		}



		//dc.PreSolve(dt);
		//dc.Solve();
		//dc.PostSolve();


		for (int32 i = 0; i < count; i++)
		{
			RigidBody* body = &bodies[i];
			body->Update(dt);
		}

#if 1
		for (int32 i = 0; i < count; i++)
		{
			RigidBody* body = &bodies[i];
			for (int32 j = 0; j < body->shape.count; j++)
			{
				DEBUGDrawOBB(CreateOBB(body->position, Vec3f(0.5f), body->orientation));

				//DEBUGDrawSphere(CreateSphere(body->position, body->shape.radius));
				//Vec3f p = body->position + RotatePointLHS(body->orientation, body->shape.points[j]);
				//Vec3f extents = Vec3f(body->shape.radius);
				//DEBUGDrawSphere(CreateSphere(p, body->shape.radius));
				//DEBUGDrawOBB(CreateOBB(p, extents, body->orientation));
			}
		}
#endif

		//for (ContactConstraint& con : cons)
		//{
		//	con.Presolve(dt);
		//	for (int32 i = 0; i < 10; i++)
		//	{
		//		con.Solve();
		//	}
		//}


	}



}

