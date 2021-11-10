#include "SolarGJK.h"
#include "core/SolarCore.h"

namespace cm
{
	struct SupportPoint
	{
		Vec3f diff;	// @NOTE: The point on the minkowski sum
		Vec3f ptA;	// @NOTE: The point on bodyA
		Vec3f ptB;	// @NOTE: The point on bodyB
	};

	struct Simplex
	{
		int32 count;
		SupportPoint points[4];

		inline void Set(const SupportPoint& first)
		{
			points[0] = first;
			count = 1;
		}

		inline void Set(const SupportPoint& first, const SupportPoint& second)
		{
			points[0] = first;
			points[1] = second;
			count = 2;
		}

		inline void Set(const SupportPoint& first, const SupportPoint& second, const SupportPoint& third)
		{
			points[0] = first;
			points[1] = second;
			points[2] = third;
			count = 3;
		}

		inline void Set(const SupportPoint& first, const SupportPoint& second, const SupportPoint& third, const SupportPoint& fourth)
		{
			points[0] = first;
			points[1] = second;
			points[2] = third;
			points[3] = fourth;
			count = 4;
		}

		inline void AddToFront(const SupportPoint& p)
		{
			Assert(count != 4, "To many points");
			points[3] = points[2];
			points[2] = points[1];
			points[1] = points[0];
			points[0] = p;
			count++;
		}
	};

	SupportPoint GetSupportPoint(RigidBody* bodyA, RigidBody* bodyB, Vec3f dir, real32 bias)
	{
		dir = Normalize(dir);

		SupportPoint point = {};

		point.ptA = bodyA->shape.GetSupportPoint(dir, bodyA->position, bodyA->orientation, bias);
		dir = -1.0f * dir;
		point.ptB = bodyB->shape.GetSupportPoint(dir, bodyB->position, bodyB->orientation, bias);

		point.diff = point.ptA - point.ptB;
		return point;
	}


#define SameDirection(a, b) (Dot(a, b) > 0.0f)

	bool LineCase(Simplex* simplex, Vec3f* dir)
	{
		SupportPoint a = simplex->points[0];
		SupportPoint b = simplex->points[1];

		Vec3f ab = b.diff - a.diff;
		Vec3f ao = -1.0f * a.diff;

		if (SameDirection(ab, ao))
		{
			*dir = Cross(Cross(ab, ao), ab);
		}
		else
		{
			*dir = ao;
			simplex->Set(a);
		}

		return true;
	}

	bool TriangleCase(Simplex* simplex, Vec3f* dir)
	{
		SupportPoint a = simplex->points[0];
		SupportPoint b = simplex->points[1];
		SupportPoint c = simplex->points[2];

		Vec3f ab = b.diff - a.diff;
		Vec3f ac = c.diff - a.diff;
		Vec3f ao = -1.0f * a.diff;
		Vec3f abc = Cross(ab, ac);

		if (SameDirection(Cross(abc, ac), ao))
		{
			if (SameDirection(ac, ao))
			{
				simplex->Set(a, c);
				*dir = Cross(Cross(ac, ao), ac);
			}
			else
			{
				simplex->Set(a, b);
				return LineCase(simplex, dir);
			}
		}
		else
		{
			if (SameDirection(Cross(ab, abc), ao))
			{
				simplex->Set(a, b);
				return LineCase(simplex, dir);
			}
			else
			{
				if (SameDirection(abc, ao))
				{
					*dir = abc;
				}
				else
				{
					simplex->Set(a, c, b);
					*dir = -1.0f * abc;
				}
			}
		}

		return true;
	}

	bool TetrahedronCase(Simplex* simplex, Vec3f* dir)
	{
		SupportPoint a = simplex->points[0];
		SupportPoint b = simplex->points[1];
		SupportPoint c = simplex->points[2];
		SupportPoint d = simplex->points[3];

		Vec3f ab = b.diff - a.diff;
		Vec3f ac = c.diff - a.diff;
		Vec3f ad = d.diff - a.diff;
		Vec3f ao = -1.0f * a.diff;

		Vec3f abc = Cross(ab, ac);
		Vec3f acd = Cross(ac, ad);
		Vec3f adb = Cross(ad, ab);

		if (SameDirection(abc, ao))
		{
			simplex->Set(a, b, c);
			return TriangleCase(simplex, dir);
		}

		if (SameDirection(acd, ao))
		{
			simplex->Set(a, c, d);
			return TriangleCase(simplex, dir);
		}

		if (SameDirection(adb, ao))
		{
			simplex->Set(a, d, b);
			return TriangleCase(simplex, dir);
		}

		return false;
	}

	bool DoSimplex(Simplex* simplex, Vec3f* dir)
	{
		switch (simplex->count)
		{
		case 2: return LineCase(simplex, dir);
		case 3: return TriangleCase(simplex, dir);
		case 4: return TetrahedronCase(simplex, dir);
		}

		Assert(0, "Should not be here");
		return false;
	}

	bool GJKIntersection(RigidBody* bodyA, RigidBody* bodyB)
	{
		Simplex simplex = {};
		simplex.AddToFront(GetSupportPoint(bodyA, bodyB, Vec3f(1.0f, 1.0f, 1.0f), 0.0f));

		Vec3f dir = Vec3f(-1.0f, -1.0f, -1.0f);
		do
		{
			SupportPoint point = GetSupportPoint(bodyA, bodyB, dir, 0.0f);
			if (Dot(point.diff, dir) < 0)
			{
				return false;
			}

			simplex.AddToFront(point);

		} while (DoSimplex(&simplex, &dir));

		return true;
	}

	/*
	================================
	EPA
	================================
	*/

	struct SupportEdge
	{
		int32 a;
		int32 b;

		bool operator == (const SupportEdge& rhs) const {
			return ((a == rhs.a && b == rhs.b) || (a == rhs.b && b == rhs.a));
		}
	};

	struct SupportTriangle
	{
		int32 a;
		int32 b;
		int32 c;

		SupportTriangle()
		{
			a = 0;
			b = 0;
			c = 0;
		}

		SupportTriangle(int32 a, int32 b, int32 c)
		{
			this->a = a;
			this->b = b;
			this->c = c;
		}

		inline void FixNormal(const ManagedArray<SupportPoint>& points, const Vec3f& p)
		{
			real32 dir = SignedDistanceFrom(points, p);
			if (dir > 0.0f)
			{
				int32 temp = a;
				a = b;
				b = temp;
			}
		}

		inline real32 SignedDistanceFrom(const ManagedArray<SupportPoint>& points, const Vec3f& p) const
		{
			// @SPEED: Cache ?
			Vec3f n = GetNormal(points);
			Vec3f dir = p - points[a].diff;
			real32 dist = Dot(dir, n);

			return dist;
		}

		inline Vec3f GetNormal(const ManagedArray<SupportPoint>& points) const
		{
			Vec3f pA = points[a].diff;
			Vec3f pB = points[b].diff;
			Vec3f pC = points[c].diff;

			Vec3f ab = pB - pA;
			Vec3f ac = pC - pA;
			// @SPEED: Do we need to actualy have a normalized normal
			Vec3f normal = Normalize(Cross(ab, ac));

			return normal;
		}

		inline bool IsValid(const Vec3f& v) const
		{
			if (v.x * 0.0f != v.x * 0.0f)
			{
				return false;
			}

			if (v.y * 0.0f != v.y * 0.0f)
			{
				return false;
			}

			if (v.z * 0.0f != v.z * 0.0f)
			{
				return false;
			}

			return true;
		}

		Vec3f GetBarycentricCoordinates(const ManagedArray<SupportPoint>& points, const Vec3f& pt) const
		{
			Vec3f s1 = points[a].diff - pt;
			Vec3f s2 = points[b].diff - pt;
			Vec3f s3 = points[c].diff - pt;

			Vec3f normal = Cross(s2 - s1, s3 - s1);
			Vec3f p0 = normal * Dot(s1, normal) / MagSqrd(normal);

			// Find the axis with the greatest projected area
			int32 idx = 0;
			real32 area_max = 0;
			for (int32 i = 0; i < 3; i++)
			{
				int32 j = (i + 1) % 3;
				int32 k = (i + 2) % 3;

				Vec2f a = Vec2f(s1[j], s1[k]);
				Vec2f b = Vec2f(s2[j], s2[k]);
				Vec2f c = Vec2f(s3[j], s3[k]);
				Vec2f ab = b - a;
				Vec2f ac = c - a;

				real32 area = ab.x * ac.y - ab.y * ac.x;
				if (area * area > area_max * area_max)
				{
					idx = i;
					area_max = area;
				}
			}

			// Project onto the appropriate axis
			int32 x = (idx + 1) % 3;
			int32 y = (idx + 2) % 3;
			Vec2f s[3];
			s[0] = Vec2f(s1[x], s1[y]);
			s[1] = Vec2f(s2[x], s2[y]);
			s[2] = Vec2f(s3[x], s3[y]);
			Vec2f p = Vec2f(p0[x], p0[y]);

			Vec3f areas;
			for (int32 i = 0; i < 3; i++)
			{
				int32 j = (i + 1) % 3;
				int32 k = (i + 2) % 3;

				Vec2f a = p;
				Vec2f b = s[j];
				Vec2f c = s[k];
				Vec2f ab = b - a;
				Vec2f ac = c - a;

				areas[i] = ab.x * ac.y - ab.y * ac.x;
			}

			Vec3f lambdas = areas / area_max;
			if (!IsValid(lambdas))
			{
				lambdas = Vec3f(1, 0, 0);
			}
			return lambdas;
		}
	};

	struct Polytope
	{
		ManagedArray<SupportPoint> points;
		ManagedArray<SupportEdge> danglingEdges;
		ManagedArray<SupportTriangle> triangles;
	};

	int32 GetClosestTriangle(const ManagedArray<SupportTriangle>& tris, const ManagedArray<SupportPoint>& points)
	{
		real32 minDistSqr = 1e10;

		int32 idx = -1;
		for (int32 i = 0; i < (int32)tris.GetCount(); i++)
		{
			SupportTriangle tri = tris[i];

			real32 dist = tri.SignedDistanceFrom(points, Vec3f(0, 0, 0));
			real32 distSqr = dist * dist;
			if (distSqr < minDistSqr)
			{
				idx = i;
				minDistSqr = distSqr;
			}
		}

		return idx;
	}

	bool ContainsPoint(const Vec3f& w, const ManagedArray<SupportTriangle>& tris, const ManagedArray<SupportPoint>& points)
	{
		const real32 epsilons = 0.001f * 0.001f;

		for (int32 i = 0; i < (int32)tris.GetCount(); i++)
		{
			SupportTriangle tri = tris[i];

			Vec3f delta = w - points[tri.a].diff;
			if (MagSqrd(delta) < epsilons)
			{
				return true;
			}

			delta = w - points[tri.b].diff;
			if (MagSqrd(delta) < epsilons)
			{
				return true;
			}

			delta = w - points[tri.c].diff;
			if (MagSqrd(delta) < epsilons)
			{
				return true;
			}
		}

		return false;
	}

	int32 RemoveTrianglesFacingPoint(const Vec3f& w, ManagedArray<SupportTriangle>& tris, const ManagedArray<SupportPoint>& points)
	{
		int32 numRemoved = 0;
		for (int32 i = 0; i < (int32)tris.GetCount(); i++)
		{
			SupportTriangle* face = &tris[i];
			real32 d = face->SignedDistanceFrom(points, w);
			if (d > 0.0)
			{
				tris.Remove(i);
				i--;
				numRemoved++;
			}
		}

		return numRemoved;
	}

	void FindDanglingEdges(const ManagedArray<SupportTriangle>& triangles, ManagedArray<SupportEdge>& danglingEdges)
	{
		danglingEdges.Clear();

		for (int32 i = 0; i < (int32)triangles.GetCount(); i++) {
			const SupportTriangle& tri = triangles[i];

			SupportEdge edges[3];
			edges[0].a = tri.a;
			edges[0].b = tri.b;

			edges[1].a = tri.b;
			edges[1].b = tri.c;

			edges[2].a = tri.c;
			edges[2].b = tri.a;

			int32 counts[3];
			counts[0] = 0;
			counts[1] = 0;
			counts[2] = 0;

			for (int32 j = 0; j < (int32)triangles.GetCount(); j++)
			{
				if (j == i)
				{
					continue;
				}

				const SupportTriangle& tri2 = triangles[j];

				SupportEdge edges2[3];
				edges2[0].a = tri2.a;
				edges2[0].b = tri2.b;

				edges2[1].a = tri2.b;
				edges2[1].b = tri2.c;

				edges2[2].a = tri2.c;
				edges2[2].b = tri2.a;

				for (int32 k = 0; k < 3; k++)
				{
					if (edges[k] == edges2[0])
					{
						counts[k]++;
					}
					if (edges[k] == edges2[1])
					{
						counts[k]++;
					}
					if (edges[k] == edges2[2])
					{
						counts[k]++;
					}
				}
			}

			// An edge that isn't shared, is dangling 
			for (int32 k = 0; k < 3; k++)
			{
				if (0 == counts[k])
				{
					danglingEdges.Add(edges[k]);
				}
			}
		}
	}

	bool EPAIntersection(RigidBody* bodyA, RigidBody* bodyB, Vec3f* ptOnA, Vec3f* ptOnB)
	{
		Simplex simplex = {};
		simplex.AddToFront(GetSupportPoint(bodyA, bodyB, Vec3f(1.0f, 1.0f, 1.0f), 0.0f));

		Vec3f dir = Vec3f(-1.0f, -1.0f, -1.0f);
		do
		{
			SupportPoint point = GetSupportPoint(bodyA, bodyB, dir, 0.0f);
			if (Dot(point.diff, dir) < 0)
			{
				return false;
			}

			simplex.AddToFront(point);

		} while (DoSimplex(&simplex, &dir));

		Assert(simplex.count == 4, "?");

		//
		// @NOTE: EPA
		//
		Polytope polytope = {};
		{
			Assert(0, "Change");
			//polytope.points = GameMemory::GetManagedArray<SupportPoint>();
			//polytope.danglingEdges = GameMemory::GetManagedArray<SupportEdge>();
			//polytope.triangles = GameMemory::GetManagedArray<SupportTriangle>();
		}
		// @TODO: Constants
		const real32 bias = 0.001f;

		// @NOTE: Get center
		Vec3f center = Vec3f(0, 0, 0);
		for (int32 i = 0; i < 4; i++)
		{
			center += simplex.points[i].diff;
		}
		center = center * 0.25f;

		// @NOTE: Build points
		for (int32 i = 0; i < 4; i++)
		{
			SupportPoint* point = &simplex.points[i];
			Vec3f dir = Normalize(point->diff - center);
			point->ptA += dir * bias;
			point->ptB += dir * bias;
			point->diff = point->ptA - point->ptB;

			polytope.points.Add(*point);
		}

		// @NOTE: Build faces
		polytope.triangles.Add(SupportTriangle(0, 1, 2));
		polytope.triangles[0].FixNormal(polytope.points, center);
		polytope.triangles.Add(SupportTriangle(0, 3, 1));
		polytope.triangles[1].FixNormal(polytope.points, center);
		polytope.triangles.Add(SupportTriangle(0, 2, 3));
		polytope.triangles[2].FixNormal(polytope.points, center);
		polytope.triangles.Add(SupportTriangle(1, 3, 2));
		polytope.triangles[3].FixNormal(polytope.points, center);

		while (true)
		{
			int32 closestFaceIndex = GetClosestTriangle(polytope.triangles, polytope.points);
			Vec3f closestFaceNormal = polytope.triangles[closestFaceIndex].GetNormal(polytope.points);
			SupportPoint newPt = GetSupportPoint(bodyA, bodyB, closestFaceNormal, bias);

			if (ContainsPoint(newPt.diff, polytope.triangles, polytope.points))
			{
				break;
			}

			real32 dist = polytope.triangles[closestFaceIndex].SignedDistanceFrom(polytope.points, newPt.diff);
			if (dist <= 0.0f)
			{
				break;
			}

			int32 newIdx = polytope.points.GetCount();
			polytope.points.Add(newPt);

			int32 numRemoved = RemoveTrianglesFacingPoint(newPt.diff, polytope.triangles, polytope.points);
			if (numRemoved == 0)
			{
				break;
			}


			FindDanglingEdges(polytope.triangles, polytope.danglingEdges);
			if (polytope.danglingEdges.GetCount() == 0)
			{
				break;
			}

			for (int32 i = 0; i < (int32)polytope.danglingEdges.GetCount(); i++)
			{
				const SupportEdge& edge = polytope.danglingEdges[i];

				SupportTriangle triangle;
				triangle.a = newIdx;
				triangle.b = edge.b;
				triangle.c = edge.a;

				// Make sure it's oriented properly
				real32 dist = triangle.SignedDistanceFrom(polytope.points, center);
				if (dist > 0.0f)
				{
					std::swap(triangle.b, triangle.c);
				}

				polytope.triangles.Add(triangle);
			}
		}

		int32 index = GetClosestTriangle(polytope.triangles, polytope.points);
		SupportTriangle* tri = &polytope.triangles[index];
		Vec3f lambdas = tri->GetBarycentricCoordinates(polytope.points, Vec3f(0.0f));

		// Get the point on shape A
		Vec3f ptA_a = polytope.points[tri->a].ptA;
		Vec3f ptB_a = polytope.points[tri->b].ptA;
		Vec3f ptC_a = polytope.points[tri->c].ptA;
		*ptOnA = ptA_a * lambdas[0] + ptB_a * lambdas[1] + ptC_a * lambdas[2];

		// Get the point on shape B
		Vec3f ptA_b = polytope.points[tri->a].ptB;
		Vec3f ptB_b = polytope.points[tri->b].ptB;
		Vec3f ptC_b = polytope.points[tri->c].ptB;
		*ptOnB = ptA_b * lambdas[0] + ptB_b * lambdas[1] + ptC_b * lambdas[2];

		return true;
	}

}