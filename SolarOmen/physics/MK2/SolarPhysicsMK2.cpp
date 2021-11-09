#include "../../Debug.h"
#include "../SolarPhysics.h"
#include "SolarRigidBodyMK2.h"
#include "SolarCollisionMK2.h"
#include "SolarConstraintMK2.h"
#include "SolarGJKMK2.h"
#include "../../SolarOmen.h"

namespace cm::MK2
{
	static int32 stepCount = 0;

	int CompareContacts(const void* p1, const void* p2) {
		Contact a = *(Contact*)p1;
		Contact b = *(Contact*)p2;

		if (a.timeOfImpact < b.timeOfImpact) {
			return -1;
		}

		if (a.timeOfImpact == b.timeOfImpact) {
			return 0;
		}

		return 1;
	}

	static Vec3f temp1 = Vec3f(0);
	static Vec3f temp2 = Vec3f(0);

	static bool32 Intersect(RigidBody* bodyA, RigidBody* bodyB, Contact* contact)
	{
		bool32 intersected = false;
		real32 bias = 0.001f;

		if (GJKIntersection(bodyA, bodyB, bias, &contact->ptOnA_WorldSpace, &contact->ptOnB_WorldSpace))
		{
			// @NOTE: There was an intersection so fill out the data
			contact->normal = Normalize(contact->ptOnB_WorldSpace - contact->ptOnA_WorldSpace);
			contact->ptOnA_WorldSpace -= contact->normal * bias;
			contact->ptOnB_WorldSpace += contact->normal * bias;

			temp1 = Vec3f(contact->ptOnA_WorldSpace.x, contact->ptOnA_WorldSpace.z, contact->ptOnA_WorldSpace.y);
			temp2 = Vec3f(contact->ptOnB_WorldSpace.x, contact->ptOnB_WorldSpace.z, contact->ptOnB_WorldSpace.y);



			contact->ptOnA_LocalSpace = bodyA->WorldSpaceToBodySpace(contact->ptOnA_WorldSpace);
			contact->ptOnB_LocalSpace = bodyB->WorldSpaceToBodySpace(contact->ptOnB_WorldSpace);
			contact->separationDistance = -1.0f * Mag(contact->ptOnA_WorldSpace - contact->ptOnB_WorldSpace);

			intersected = true;
		}
		else
		{
			// @NOTE: There was no intersection so get closest distances
			GJKClosestPoints(bodyA, bodyB, &contact->ptOnA_WorldSpace, &contact->ptOnB_WorldSpace);

			contact->ptOnA_LocalSpace = bodyA->WorldSpaceToBodySpace(contact->ptOnA_WorldSpace);
			contact->ptOnB_LocalSpace = bodyB->WorldSpaceToBodySpace(contact->ptOnB_WorldSpace);
			contact->separationDistance = Mag(contact->ptOnA_WorldSpace - contact->ptOnB_WorldSpace);

			intersected = false;
		}

		return intersected;
	}

	static bool ConservativeAdvance(RigidBody* bodyA, RigidBody* bodyB, real32 dt, Contact* contact)
	{
		real32 toi = 0.0f;
		int32 iterationCount = 0;

		// @NOTE: We have some time left
		while (dt > 0.0f)
		{
			if (Intersect(bodyA, bodyB, contact))
			{
				// @NOTE: Bodies are intersecting so we're done and just undo any changes to the body
				contact->timeOfImpact = toi;
				bodyA->Update(-toi);
				bodyB->Update(-toi);
				return true;
			}

			iterationCount++;
			if (iterationCount > 10)
			{
				break;
			}

			Vec3f ab = Normalize(contact->ptOnB_WorldSpace - contact->ptOnA_WorldSpace);

			Vec3f relativeVelocity = bodyA->linearVelocity - bodyB->linearVelocity;
			real32 angularSpeedA = bodyA->shape.GetFastestLinearSpeed(bodyA->angularVelocity, ab);
			real32 angularSpeedB = bodyB->shape.GetFastestLinearSpeed(bodyB->angularVelocity, -1.0f * ab);

			real32 orthoSpeed = Dot(relativeVelocity, ab) + angularSpeedA + angularSpeedB;

			// @NOTE: Bodies are seperating
			if (orthoSpeed <= 0.0f)
			{
				break;
			}

			// @NOTE: ratio of out quickly objects will intersect eg 10 / 1 -> 10 meaning possible too far to possible intersect 
			real32 timeToGo = contact->separationDistance / orthoSpeed;

			if (timeToGo > dt)
			{
				break;
			}

			// @NOTE: Remove some time
			dt -= timeToGo;
			// @NOTE: Keep track of how much time has elasped
			toi += timeToGo;

			// @NOTE: Step the two bodies forward
			bodyA->Update(timeToGo);
			bodyB->Update(timeToGo);
		}

		// @NOTE: Unwind the clock
		bodyA->Update(-toi);
		bodyB->Update(-toi);

		return false;
	}

	void StepSimulation(std::vector<RigidBody>& bodies)
	{
		real32 deltaTime = 0.016f / 2.0f;
		DEBUGLog("========================");
		DEBUGLog(CString().Add(stepCount++));

		if (stepCount == 98)
		{
			int a = 2;
		}

		for (RigidBody& body : bodies)
		{
			real32 m = 1.0f / body.invMass;
			Vec3f grav = Vec3f(0, 0, -10) * m * deltaTime;
			body.ApplyImpulseLinear(grav);
		}

		std::vector<Contact> contacts;

		for (int32 i = 0; i < bodies.size(); i++)
		{
			RigidBody* bodyA = &bodies[i];

			for (int32 j = i + 1; j < bodies.size(); j++)
			{
				RigidBody* bodyB = &bodies[j];

				// @TODO: If both has infinite mass don't do a collision check maybe ??
				if (bodyA->invMass == 0.0f && bodyB->invMass == 0.0f)
				{
					continue;
				}

#if 1
				Contact contact = {};
				if (ConservativeAdvance(bodyA, bodyB, deltaTime, &contact))
				{
					contact.bodyA = bodyA;
					contact.bodyB = bodyB;

					contacts.push_back(contact);

					//DEBUGLog("CONTANCT INFO");
					//DEBUGLog(CString("APos: ").Add(ToString(bodyA->position)));
					//DEBUGLog(CString("BPos: ").Add(ToString(bodyB->position)));
					//DEBUGLog(CString("Normal: ").Add(ToString(contact.normal)));
					//DEBUGLog(CString("PtA: ").Add(ToString(contact.ptOnA_WorldSpace)));
					//DEBUGLog(CString("PtB: ").Add(ToString(contact.ptOnB_WorldSpace)));
					//DEBUGLog(CString("Sep: ").Add(contact.separationDistance));
					//DEBUGLog(CString("TOI: ").Add(contact.timeOfImpact));
				}

#else
				Manifold info = {};
				Sphere cA = CreateSphere(bodyA->position, bodyA->shape.radius);
				Sphere cB = CreateSphere(bodyB->position, bodyB->shape.radius);

				if (SweepManifoldSphere(
					cA, bodyA->linearVelocity,
					cB, bodyB->linearVelocity,
					deltaTime, &info))
				{
					bodyA->Update(info.toi);
					bodyB->Update(info.toi);

					info.pt_onA_Body = bodyA->WorldSpaceToBodySpace(info.pt_onA_World);
					info.pt_onB_Body = bodyB->WorldSpaceToBodySpace(info.pt_onB_World);

					info.normal = Normalize(bodyA->position - bodyB->position);


					bodyA->Update(-info.toi);
					bodyB->Update(-info.toi);

					Vec3f ab = bodyA->position - bodyB->position;
					info.seperationDistance = Mag(ab) - (cA.data.w + cB.data.w);

					Contact con = {};
					con.bodyA = bodyA;
					con.bodyB = bodyB;
					con.normal = info.normal;
					con.ptOnA_WorldSpace = info.pt_onA_World;
					con.ptOnB_WorldSpace = info.pt_onB_World;
					con.ptOnA_LocalSpace = info.pt_onA_Body;
					con.ptOnB_LocalSpace = info.pt_onB_Body;
					con.timeOfImpact = info.toi;
					con.separationDistance = info.seperationDistance;

					contacts.push_back(con);

					//DEBUGLog("CONTANCT INFO");
					//DEBUGLog(ToString(info.normal));
					//DEBUGLog(ToString(info.pt_onA_World));
					//DEBUGLog(ToString(info.pt_onB_World));
					//DEBUGLog(CString().Add(info.seperationDistance));
					//DEBUGLog(CString().Add(info.toi));
				}
#endif

			}
		}

		if (contacts.size() > 1) {
			qsort(contacts.data(), contacts.size(), sizeof(Contact), CompareContacts);
		}

		static std::vector<ConstraintDistance>joints(4);
		joints[0].bodyA = &bodies[0];
		joints[0].bodyB = &bodies[1];

		joints[1].bodyA = &bodies[1];
		joints[1].bodyB = &bodies[2];

		joints[2].bodyA = &bodies[2];
		joints[2].bodyB = &bodies[3];

		joints[3].bodyA = &bodies[3];
		joints[3].bodyB = &bodies[4];

		static bool doOnce = true;
		if (doOnce)
		{
			joints[0].anchorA = bodies[0].WorldSpaceToBodySpace(bodies[0].position);
			joints[0].anchorB = bodies[1].WorldSpaceToBodySpace(bodies[0].position);

			joints[1].anchorA = bodies[1].WorldSpaceToBodySpace(bodies[1].position);
			joints[1].anchorB = bodies[2].WorldSpaceToBodySpace(bodies[1].position);

			joints[2].anchorA = bodies[2].WorldSpaceToBodySpace(bodies[2].position);
			joints[2].anchorB = bodies[3].WorldSpaceToBodySpace(bodies[2].position);

			joints[3].anchorA = bodies[3].WorldSpaceToBodySpace(bodies[3].position);
			joints[3].anchorB = bodies[4].WorldSpaceToBodySpace(bodies[3].position);

			doOnce = false;
		}

		for (int32 i = 0; i < joints.size(); i++)
		{
			joints[i].PreSolve(deltaTime);
		}

		for (int32 it = 0; it < 10; it++)
		{
			for (int32 i = 0; i < joints.size(); i++)
			{
				joints[i].Solve();
			}
		}

		for (int32 i = 0; i < joints.size(); i++)
		{
			joints[i].PostSolve();
		}


		real32 accumulatedTime = 0.0f;
		for (int i = 0; i < contacts.size(); i++) {
			Contact& contact = contacts[i];
			const real32 dt = contact.timeOfImpact - accumulatedTime;

			// Position update
			for (int32 j = 0; j < bodies.size(); j++) {
				bodies[j].Update(dt);
			}

			contact.Resolve();
			accumulatedTime += dt;
		}

		const real32 timeRemaining = deltaTime - accumulatedTime;
		if (timeRemaining > 0.0f) {
			for (int i = 0; i < bodies.size(); i++) {
				bodies[i].Update(timeRemaining);

				if (bodies[i].invMass != 0.0f)
				{
					//DEBUGLog("UPDATE INFO");
					//DEBUGLog(ToString(bodies[i].position));
					//DEBUGLog(ToString(bodies[i].linearVelocity));
					//DEBUGLog(ToString(bodies[i].angularVelocity));
					//DEBUGLog(ToString(bodies[i].orientation));
				}
			}
		}

		DEBUGLog("========================");
	}

	void UpdatePhsyics_(GameState* gs, TransientState* ts, Input* input)
	{
		DEBUGDrawPoint(temp1);
		DEBUGDrawPoint(temp2);

		if (input->f7 || IsKeyJustDown(input, f8))
		{

			// @NOTE: Convert to right handed coords system
			std::vector<RigidBody> bodies;
			std::vector<Entity*> entities;
			gs->BeginEntityLoop();
			while (Entity* entity = gs->GetNextEntity())
			{
				if (entity->IsPhsyicsEnabled())
				{
					RigidBody body = {};
					body.position.x = entity->transform.position.x;
					body.position.y = entity->transform.position.z;
					body.position.z = entity->transform.position.y;

					body.orientation.x = -entity->transform.orientation.x;
					body.orientation.y = -entity->transform.orientation.z;
					body.orientation.z = -entity->transform.orientation.y;
					body.orientation.w = entity->transform.orientation.w;

					body.linearVelocity = entity->rigidBody.linearVelocity;
					body.angularVelocity = entity->rigidBody.angularVelocity;
					body.invMass = entity->rigidBody.invMass;
					body.elasticity = entity->rigidBody.elasticity;
					body.friction = entity->rigidBody.friction;

#if 0 
					body.shape = PhysicsShape();
					body.shape.radius = 0.5f;
					body.shape.type = PhysicsShapeType::SHPERE;
#else
					body.shape = PhysicsShape::CreateBox();
#endif
					bodies.push_back(body);
					entities.push_back(entity);
				}
			}

			{
				RigidBody body = {};
				body.invMass = 1.0f;
				body.shape = PhysicsShape::CreateBox();
				//body.ApplyImpulse(Vec3f(0, 1, 1), Vec3f(0, 0, 10));
				body.orientation = Normalize(Quatf(20.32f, 31.23f, 2.2384f, 2924.3164f));

				body.Update(0.016f);
				body.Update(0.016f);
				body.Update(0.016f);
				body.Update(0.016f);

				int a = 2;
			}

			//std::swap(bodies[0], bodies[1]);
			//std::swap(entities[0], entities[1]);

			StepSimulation(bodies);

			for (int32 i = 0; i < bodies.size(); i++)
			{
				const RigidBody& body = bodies.at(i);
				Entity* entity = entities.at(i);

				entity->transform.position.x = body.position.x;
				entity->transform.position.y = body.position.z;
				entity->transform.position.z = body.position.y;

				entity->transform.orientation.x = -body.orientation.x;
				entity->transform.orientation.y = -body.orientation.z;
				entity->transform.orientation.z = -body.orientation.y;
				entity->transform.orientation.w = body.orientation.w;

				entity->rigidBody.linearVelocity = body.linearVelocity;
				entity->rigidBody.angularVelocity = body.angularVelocity;

				OBB box = CreateOBB(body.position, Vec3f(0.5f), body.orientation);

				DEBUGDrawOBB(box);
			}
		}
	}
}