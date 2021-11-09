#include "SolarCollisionMK2.h"

namespace cm::MK2
{
	void Contact::Resolve()
	{
		const Vec3f ptOnA = bodyA->BodySpaceToWorldSpace(ptOnA_LocalSpace);
		const Vec3f ptOnB = bodyB->BodySpaceToWorldSpace(ptOnB_LocalSpace);

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

		Vec3f t1 = Cross(rb, relativeVelTang);
		Vec3f t2 = (t1 * invWorldInertiaB);

		const Vec3f inertiaA = Cross((Cross(ra, relativeVelTang) * invWorldInertiaA), ra);
		const Vec3f inertiaB = Cross(t2, rb);
		const real32 invInertia = Dot(inertiaA + inertiaB, relativeVelTang);

		// Calculate the tangential impulse for friction
		const real32 reducedMass = 1.0f / (bodyA->invMass + bodyB->invMass + invInertia);
		const Vec3f impulseFriction = velTang * reducedMass * friction;

		// Apply kinetic friction
		bodyA->ApplyImpulse(ptOnA, impulseFriction * -1.0f);
		bodyB->ApplyImpulse(ptOnB, impulseFriction * 1.0f);

		//
		// Let's also move our colliding objects to just outside of each other (projection method)
		//
		if (0.0f == timeOfImpact) {
			const Vec3f ds = ptOnB - ptOnA;

			const real32 tA = invMassA / (invMassA + invMassB);
			const real32 tB = invMassB / (invMassA + invMassB);

			bodyA->position += ds * tA;
			bodyB->position -= ds * tB;
		}
	}
}