#include "SolarGJKMK2.h"
#include "../../Debug.h"


namespace cm::MK2
{
	static Vec2f OriginCheck1Simplex(const Vec3f& s1, const Vec3f& s2)
	{
		Vec3f ab = s2 - s1;
		Vec3f ap = Vec3f(0.0f) - s1;
		Vec3f p0 = s1 + ab * Dot(ab, ap) / MagSqrd(ab);

		real32 max = 0;
		int32 index = 0;
		for (int32 i = 0; i < 3; i++)
		{
			real32 mu = s2[i] - s1[i];
			if (mu * mu > max * max)
			{
				max = mu;
				index = i;
			}
		}

		real32 a = s1[index];
		real32 b = s2[index];
		real32 p = p0[index];

		real32 c1 = p - a;
		real32 c2 = b - p;

		// if p is between [a,b]
		if ((p > a && p < b) || (p > b && p < a)) {
			Vec2f lambdas = Vec2f(c2 / max, c1 / max);
			return lambdas;
		}

		// if p is on the far side of a
		if ((a <= b && p <= a) || (a >= b && p >= a)) {
			return Vec2f(1.0f, 0.0f);
		}

		// p must be on the far side of b
		return Vec2f(0.0f, 1.0f);
	}

	static int CompareSigns(real32 a, real32 b) {
		if (a > 0.0f && b > 0.0f) {
			return 1;
		}
		if (a < 0.0f && b < 0.0f) {
			return 1;
		}
		return 0;
	}

	static Vec3f OriginCheck2Simplex(const Vec3f& s1, const Vec3f& s2, const Vec3f& s3)
	{
		Vec3f normal = Cross(s2 - s1, s3 - s1);
		Vec3f p0 = normal * Dot(s1, normal) / MagSqrd(normal);

		int32 index = 0;
		real32 max = 0;

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
			if (area * area > max * max) {
				index = i;
				max = area;
			}
		}

		// Project onto the appropriate axis
		int32 x = (index + 1) % 3;
		int32 y = (index + 2) % 3;
		Vec2f s[3] = {};
		s[0] = Vec2f(s1[x], s1[y]);
		s[1] = Vec2f(s2[x], s2[y]);
		s[2] = Vec2f(s3[x], s3[y]);
		Vec2f p = Vec2f(p0[x], p0[y]);

		// Get the sub-areas of the triangles formed from the projected origin and the edges
		Vec3f areas = {};
		for (int32 i = 0; i < 3; i++) {
			int32 j = (i + 1) % 3;
			int32 k = (i + 2) % 3;

			Vec2f a = p;
			Vec2f b = s[j];
			Vec2f c = s[k];
			Vec2f ab = b - a;
			Vec2f ac = c - a;

			areas[i] = ab.x * ac.y - ab.y * ac.x;
		}


		// If the projected origin is inside the triangle, then return the barycentric points
		if (CompareSigns(max, areas[0]) > 0 && CompareSigns(max, areas[1]) > 0 && CompareSigns(max, areas[2]) > 0) {
			Vec3f lambdas = areas / max;
			return lambdas;
		}

		// If we make it here, then we need to project onto the edges and determine the closest point
		real32 dist = 1e10f;
		Vec3f lambdas = Vec3f(1, 0, 0);
		for (int32 i = 0; i < 3; i++) {
			int32 k = (i + 1) % 3;
			int32 l = (i + 2) % 3;

			Vec3f edgesPts[3] = {};
			edgesPts[0] = s1;
			edgesPts[1] = s2;
			edgesPts[2] = s3;

			Vec2f lambdaEdge = OriginCheck1Simplex(edgesPts[k], edgesPts[l]);
			Vec3f pt = edgesPts[k] * lambdaEdge[0] + edgesPts[l] * lambdaEdge[1];

			if (MagSqrd(pt) < dist) {
				dist = MagSqrd(pt);
				lambdas[i] = 0;
				lambdas[k] = lambdaEdge[0];
				lambdas[l] = lambdaEdge[1];
			}
		}

		return lambdas;
	}

	static Vec4f OriginCheck3Simplex(const Vec3f& s1, const Vec3f& s2, const Vec3f& s3, const Vec3f& s4)
	{
		Mat4f M = {};
		M.row0 = Vec4f(s1.x, s2.x, s3.x, s4.x);
		M.row1 = Vec4f(s1.y, s2.y, s3.y, s4.y);
		M.row2 = Vec4f(s1.z, s2.z, s3.z, s4.z);
		M.row3 = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);

		Vec4f C4 = {};
		C4[0] = Cofactor(M, 3, 0);
		C4[1] = Cofactor(M, 3, 1);
		C4[2] = Cofactor(M, 3, 2);
		C4[3] = Cofactor(M, 3, 3);

		real32 detM = C4[0] + C4[1] + C4[2] + C4[3];

		// If the barycentric coordinates put the origin inside the simplex, then return them
		if (CompareSigns(detM, C4[0]) > 0 && CompareSigns(detM, C4[1]) > 0 && CompareSigns(detM, C4[2]) > 0 && CompareSigns(detM, C4[3]) > 0) {
			Vec4 lambdas = C4 * (1.0f / detM);
			return lambdas;
		}

		// If we get here, then we need to project the origin onto the faces and determine the closest one
		Vec4f lambdas = {};
		real32 dist = 1e10f;
		for (int32 i = 0; i < 4; i++) {
			int32 j = (i + 1) % 4;
			int32 k = (i + 2) % 4;

			Vec3f facePts[4];
			facePts[0] = s1;
			facePts[1] = s2;
			facePts[2] = s3;
			facePts[3] = s4;

			Vec3f lambdasFace = OriginCheck2Simplex(facePts[i], facePts[j], facePts[k]);
			Vec3f pt = facePts[i] * lambdasFace[0] + facePts[j] * lambdasFace[1] + facePts[k] * lambdasFace[2];
			if (MagSqrd(pt) < dist) {
				dist = MagSqrd(pt);
				lambdas = Vec4f(0);
				lambdas[i] = lambdasFace[0];
				lambdas[j] = lambdasFace[1];
				lambdas[k] = lambdasFace[2];
			}
		}

		return lambdas;
	}

	struct CheckSimplexResult
	{
		bool intersect;
		Vec3f newDir;
		Vec4f lambdas;
	};

	struct GJKPoint
	{
		Vec3f diff;
		Vec3f ptA;
		Vec3f ptB;

		const GJKPoint& operator = (const GJKPoint& rhs) {
			diff = rhs.diff;
			ptA = rhs.ptA;
			ptB = rhs.ptB;
			return *this;
		}

		bool operator == (const GJKPoint& rhs) const {
			return ((ptA == rhs.ptA) && (ptB == rhs.ptB) && (diff == rhs.diff));
		}

		void Print(int32 index) const
		{
			DEBUGLog(CString().Add(index));
			DEBUGLog(CString().Add(ToString(diff)));
			DEBUGLog(CString().Add(ToString(ptA)));
			DEBUGLog(CString().Add(ToString(ptB)));
		}
	};

	static CheckSimplexResult CheckSimplex(GJKPoint* pts, int32 num)
	{
		CheckSimplexResult result = {};
		const float epsilonf = 0.0001f * 0.0001f;
		switch (num)
		{
		default:
		case 2:
		{
			Vec2f lambdas = OriginCheck1Simplex(pts[0].diff, pts[1].diff);
			Vec3f v = Vec3f(0.0f);

			v += pts[0].diff * lambdas[0];
			v += pts[1].diff * lambdas[1];

			result.newDir = v * -1.0f;
			result.intersect = (MagSqrd(v) < epsilonf);
			result.lambdas[0] = lambdas[0];
			result.lambdas[1] = lambdas[1];
		}break;

		case 3:
		{
			Vec3f lambdas = OriginCheck2Simplex(pts[0].diff, pts[1].diff, pts[2].diff);
			Vec3f v = Vec3f(0.0f);

			v += pts[0].diff * lambdas[0];
			v += pts[1].diff * lambdas[1];
			v += pts[2].diff * lambdas[2];

			result.newDir = v * -1.0f;
			result.intersect = (MagSqrd(v) < epsilonf);
			result.lambdas[0] = lambdas[0];
			result.lambdas[1] = lambdas[1];
			result.lambdas[2] = lambdas[2];
		}break;

		case 4:
		{
			Vec4f lambdas = OriginCheck3Simplex(pts[0].diff, pts[1].diff, pts[2].diff, pts[3].diff);
			Vec3f v = Vec3f(0.0f);

			v += pts[0].diff * lambdas[0];
			v += pts[1].diff * lambdas[1];
			v += pts[2].diff * lambdas[2];
			v += pts[3].diff * lambdas[3];

			result.newDir = v * -1.0f;
			result.intersect = (MagSqrd(v) < epsilonf);
			result.lambdas[0] = lambdas[0];
			result.lambdas[1] = lambdas[1];
			result.lambdas[2] = lambdas[2];
			result.lambdas[3] = lambdas[3];
		}break;
		}

		return result;
	}

	static GJKPoint GetSupportPoint(RigidBody* bodyA, RigidBody* bodyB, Vec3f dir, const real32 bias)
	{
		dir = Normalize(dir);

		GJKPoint point = {};

		point.ptA = bodyA->shape.GetSupport(dir, bodyA->position, bodyA->orientation, bias);
		point.ptB = bodyB->shape.GetSupport(-1.0f * dir, bodyB->position, bodyB->orientation, bias);

		point.diff = point.ptA - point.ptB;

		return point;
	}

	static bool32 HasPoint(const GJKPoint simplexPoints[4], const GJKPoint& newPt)
	{
		const float precision = 1e-6f;
		for (int32 i = 0; i < 4; i++)
		{
			Vec3 delta = simplexPoints[i].diff - newPt.diff;
			if (MagSqrd(delta) < precision * precision) {
				return true;
			}
		}

		return false;
	}

	static int32 SortValidPointsAndLambdas(GJKPoint simplexPoints[4], Vec4f* lambdas)
	{
		int32 count = 0;
		GJKPoint validPts[4] = {};
		Vec4f validLambdas = Vec4f(0.0f);
		for (int32 i = 0; i < 4; i++)
		{
			if ((*lambdas)[i] != 0.0f) {

				validPts[count] = simplexPoints[i];
				validLambdas[count] = (*lambdas)[i];
				count++;
			}
		}

		for (int32 i = 0; i < 4; i++)
		{
			simplexPoints[i] = validPts[i];
			(*lambdas)[i] = validLambdas[i];
		}

		return count;
	}

	bool GJKIntersection(RigidBody* bodyA, RigidBody* bodyB)
	{
		//PROFILE_FUNCTION();
		const Vec3f origin = Vec3f(0, 0, 0);

		//DEBUGDrawOBB(GetEntityCollider(bodyA, 0));
		//DEBUGDrawOBB(GetEntityCollider(bodyB, 0));

		int32 numPts = 1;
		GJKPoint simplexPoints[4] = {};
		simplexPoints[0] = GetSupportPoint(bodyA, bodyB, Vec3f(1, 1, 1), 0.0f);

		real32 closestDist = 1e10f;
		bool doesContainOrigin = false;
		Vec3 newDir = simplexPoints[0].diff * -1.0f;

		do
		{
			// @NOTE: Get the new support point
			GJKPoint currentPoint = GetSupportPoint(bodyA, bodyB, newDir, 0.0f);

			// @NOTE: If the new point is the same as a previous point, then we can't expand any further
			if (HasPoint(simplexPoints, currentPoint))
			{
				break;
			}

			simplexPoints[numPts] = currentPoint;
			numPts++;

			// @NOTE: If this new point hasn't moved passed the origin, then the
			// @NOTE: origin cannot be in the set. And therefore there is no collision.
			if (Dot(newDir, (currentPoint.diff - origin)) < 0.0f)
			{
				break;
			}

			// @NOTE: Now project the origin onto the simplex
			CheckSimplexResult check = CheckSimplex(simplexPoints, numPts);
			if (check.intersect)
			{
				doesContainOrigin = check.intersect;
				break;
			}

			newDir = check.newDir;


			// @NOTE: Check that the new projection of the origin onto the simplex is closer than the previous
			real32 dist = MagSqrd(newDir);
			if (dist >= closestDist)
			{
				break;
			}
			closestDist = dist;

			// @NOTE: Use the lambdas that support the new search direction, and invalidate any points that don't support it
			numPts = SortValidPointsAndLambdas(simplexPoints, &check.lambdas);

			doesContainOrigin = (4 == numPts);

		} while (!doesContainOrigin);


		return doesContainOrigin;
	}

	void GJKClosestPoints(RigidBody* bodyA, RigidBody* bodyB, Vec3f* ptOnA, Vec3f* ptOnB)
	{
		const Vec3 origin(0.0f);

		real32 bias = 0.0f;
		real32 closestDist = 1e10f;

		int32 numPts = 1;
		GJKPoint simplexPoints[4] = {};

		simplexPoints[0] = GetSupportPoint(bodyA, bodyB, Vec3f(1, 1, 1), bias);

		Vec4f lambdas = Vec4f(1, 0, 0, 0);
		Vec3f newDir = simplexPoints[0].diff * -1.0f;
		do {
			// @NOTE: Get the new point to check on
			GJKPoint newPt = GetSupportPoint(bodyA, bodyB, newDir, bias);

			// @NOTE: If the new point is the same as a previous point, then we can't expand any further
			if (HasPoint(simplexPoints, newPt)) {
				break;
			}

			// @NOTE: Add point and get new search direction
			simplexPoints[numPts] = newPt;
			numPts++;

			CheckSimplexResult check = CheckSimplex(simplexPoints, numPts);
			newDir = check.newDir;
			lambdas = check.lambdas;

			numPts = SortValidPointsAndLambdas(simplexPoints, &lambdas);

			// @NOTE: Check that the new projection of the origin onto the simplex is closer than the previous
			real32 dist = MagSqrd(newDir);
			if (dist >= closestDist) {
				break;
			}
			closestDist = dist;
		} while (numPts < 4);

		*ptOnA = Vec3f(0);
		*ptOnB = Vec3f(0);

		for (int32 i = 0; i < 4; i++) {
			*ptOnA += simplexPoints[i].ptA * lambdas[i];
			*ptOnB += simplexPoints[i].ptB * lambdas[i];
		}
	}

	///
	/// 
	/// EPA 
	/// 
	///
	/// 
	/// 

	static inline void GetOrtho(Vec3f a, Vec3f* u, Vec3f* v)
	{
		a = Normalize(a);

		Vec3f w = (a.z * a.z > 0.9f * 0.9f) ? Vec3f(1, 0, 0) : Vec3f(0, 0, 1);

		*u = Normalize(Cross(w, a));
		*v = Normalize(Cross(a, *u));
		*u = Normalize(Cross(*v, a));
	}

	struct EPATri
	{
		int32 a;
		int32 b;
		int32 c;

		// @SPEED: Cache ?
		inline Vec3f GetNormal(const std::vector<GJKPoint>& points) const
		{
			const Vec3f& a = points[this->a].diff;
			const Vec3f& b = points[this->b].diff;
			const Vec3f& c = points[this->c].diff;

			Vec3f ab = b - a;
			Vec3f ac = c - a;
			Vec3f normal = Normalize(Cross(ab, ac));

			return normal;
		}
	};

	static real32 SignedDistanceToTriangle(const EPATri& tri, const Vec3f& pt, const std::vector<GJKPoint>& points)
	{
		Vec3f normal = tri.GetNormal(points);
		Vec3f a = points[tri.a].diff;
		Vec3f a2pt = pt - a;
		real32 dist = Dot(normal, a2pt);

		return dist;
	}

	struct EPAEdge
	{
		int32 a;
		int32 b;

		bool operator == (const EPAEdge& rhs) const {
			return ((a == rhs.a && b == rhs.b) || (a == rhs.b && b == rhs.a));
		}
	};

	int32 EPAClosestTriangle(const std::vector<EPATri>& triangles, const std::vector<GJKPoint>& points)
	{
		real32 minDistSqr = 1e10f;

		int32 idx = -1;
		for (int32 i = 0; i < triangles.size(); i++) {
			const EPATri& tri = triangles[i];

			real32 dist = SignedDistanceToTriangle(tri, Vec3f(0.0f), points);
			real32 distSqr = dist * dist;

			if (distSqr < minDistSqr)
			{
				idx = i;
				minDistSqr = distSqr;
			}
		}

		return idx;
	}

	bool EPAHasPoint(const Vec3f& w, const std::vector<EPATri>& triangles, const std::vector<GJKPoint>& points)
	{
		real32 epsilons = 0.001f * 0.001f;
		Vec3f delta;

		for (int32 i = 0; i < triangles.size(); i++)
		{
			const EPATri& tri = triangles[i];

			delta = w - points[tri.a].diff;
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

	static int32 RemoveTrianglesFacingPoint(const Vec3f& pt, std::vector<EPATri>& triangles, const std::vector<GJKPoint>& points)
	{
		int32 numRemoved = 0;
		for (int32 i = 0; i < triangles.size(); i++)
		{
			const EPATri& tri = triangles[i];

			real32 dist = SignedDistanceToTriangle(tri, pt, points);
			if (dist > 0.0f)
			{
				// This triangle faces the point. Remove it.
				triangles.erase(triangles.begin() + i);
				i--;
				numRemoved++;
			}
		}

		return numRemoved;
	}

	static void FindDanglingEdges(std::vector<EPAEdge>& danglingEdges, const std::vector<EPATri>& triangles) {
		danglingEdges.clear();
		for (int32 i = 0; i < triangles.size(); i++)
		{
			const EPATri& tri = triangles[i];

			EPAEdge edges[3] = {};
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

			for (int32 j = 0; j < triangles.size(); j++)
			{
				if (j == i) {
					continue;
				}

				const EPATri& tri2 = triangles[j];

				EPAEdge edges2[3];
				edges2[0].a = tri2.a;
				edges2[0].b = tri2.b;

				edges2[1].a = tri2.b;
				edges2[1].b = tri2.c;

				edges2[2].a = tri2.c;
				edges2[2].b = tri2.a;

				for (int32 k = 0; k < 3; k++) {
					if (edges[k] == edges2[0]) {
						counts[k]++;
					}
					if (edges[k] == edges2[1]) {
						counts[k]++;
					}
					if (edges[k] == edges2[2]) {
						counts[k]++;
					}
				}
			}

			// An edge that isn't shared, is dangling 
			for (int32 k = 0; k < 3; k++) {
				if (0 == counts[k]) {
					danglingEdges.push_back(edges[k]);
				}
			}
		}
	}

	inline static bool EPALambdasValid(const Vec3f& lam)
	{
		if (lam.x * 0.0f != lam.x * 0.0f) {
			return false;
		}

		if (lam.y * 0.0f != lam.y * 0.0f) {
			return false;
		}

		if (lam.z * 0.0f != lam.z * 0.0f) {
			return false;
		}

		return true;
	}

	static Vec3f EPABarycentricCoordinates(Vec3f s1, Vec3f s2, Vec3f s3, const Vec3f& pt) {
		s1 = s1 - pt;
		s2 = s2 - pt;
		s3 = s3 - pt;

		Vec3f normal = Cross((s2 - s1), s3 - s1);
		Vec3f p0 = normal * Dot(s1, normal) / MagSqrd(normal);

		// Find the axis with the greatest projected area
		int32 idx = 0;
		real32 area_max = 0;
		for (int32 i = 0; i < 3; i++) {
			int32 j = (i + 1) % 3;
			int32 k = (i + 2) % 3;

			Vec2f a = Vec2f(s1[j], s1[k]);
			Vec2f b = Vec2f(s2[j], s2[k]);
			Vec2f c = Vec2f(s3[j], s3[k]);
			Vec2f ab = b - a;
			Vec2f ac = c - a;

			real32 area = ab.x * ac.y - ab.y * ac.x;
			if (area * area > area_max * area_max) {
				idx = i;
				area_max = area;
			}
		}

		// Project onto the appropriate axis
		int32 x = (idx + 1) % 3;
		int32 y = (idx + 2) % 3;
		Vec2f s[3] = {};
		s[0] = Vec2f(s1[x], s1[y]);
		s[1] = Vec2f(s2[x], s2[y]);
		s[2] = Vec2f(s3[x], s3[y]);
		Vec2f p = Vec2f(p0[x], p0[y]);

		// Get the sub-areas of the triangles formed from the projected origin and the edges
		Vec3f areas = {};
		for (int32 i = 0; i < 3; i++) {
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
		if (!EPALambdasValid(lambdas)) {
			lambdas = Vec3f(1, 0, 0);
		}

		return lambdas;
	}

	static real32 EPAExpand(RigidBody* bodyA, RigidBody* bodyB, const float bias, const GJKPoint simplexPoints[4], Vec3f* ptOnA, Vec3f* ptOnB)
	{
		std::vector<GJKPoint> points;
		std::vector<EPATri> triangles;
		std::vector<EPAEdge> danglingEdges;

		// @SPEED: A repeat of a previous operation
		Vec3 center(0.0f);
		for (int i = 0; i < 4; i++)
		{
			points.push_back(simplexPoints[i]);
			center += simplexPoints[i].diff;
			//LOG(ToString(simplexPoints[i].diff).GetCStr());
		}
		center = center * 0.25f;


		simplexPoints[0].Print(0);
		simplexPoints[1].Print(1);
		simplexPoints[2].Print(2);
		simplexPoints[3].Print(3);

		// @NOTE: Build the triangles
		for (int32 i = 0; i < 4; i++)
		{
			int32 j = (i + 1) % 4;
			int32 k = (i + 2) % 4;

			EPATri tri = {};
			tri.a = i;
			tri.b = j;
			tri.c = k;

			int32 unusedPt = (i + 3) % 4;
			real32 dist = SignedDistanceToTriangle(tri, points[unusedPt].diff, points);

			// The unused point is always on the negative/inside of the triangle.. make sure the normal points away
			if (dist > 0.0f) {
				std::swap(tri.a, tri.b);
			}

			triangles.push_back(tri);
		}

		// @NOTE: Expand the simplex to find the closest face of the CSO to the origin
		while (1)
		{
			int32 idx = EPAClosestTriangle(triangles, points);
			Vec3f normal = triangles[idx].GetNormal(points);

			GJKPoint newPt = GetSupportPoint(bodyA, bodyB, normal, bias);

			// @NOTE: if w already exists, then just stop because it means we can't expand any further
			if (EPAHasPoint(newPt.diff, triangles, points)) {
				break;
			}

			real32 dist = SignedDistanceToTriangle(triangles[idx], newPt.diff, points);
			// @NOTE: can't expand
			if (dist <= 0.0f) {
				break;
			}

			int32 newIdx = (int32)points.size();
			points.push_back(newPt);

			// @NOTE: Remove Triangles that face this point
			int32 numRemoved = RemoveTrianglesFacingPoint(newPt.diff, triangles, points);
			if (0 == numRemoved) {
				break;
			}

			// Find Dangling Edges
			danglingEdges.clear();
			FindDanglingEdges(danglingEdges, triangles);
			if (0 == danglingEdges.size()) {
				break;
			}

			// In theory the edges should be a proper CCW order
			// So we only need to add the new point as 'a' in order
			// to create new triangles that face away from origin
			for (int32 i = 0; i < danglingEdges.size(); i++)
			{
				EPAEdge& edge = danglingEdges[i];

				EPATri triangle = {};
				triangle.a = newIdx;
				triangle.b = edge.b;
				triangle.c = edge.a;

				// Make sure it's oriented properly
				real32 dist = SignedDistanceToTriangle(triangle, center, points);
				if (dist > 0.0f) {
					std::swap(triangle.b, triangle.c);
				}

				triangles.push_back(triangle);
			}
		}

		// Get the projection of the origin on the closest triangle
		const int32 idx = EPAClosestTriangle(triangles, points);
		const EPATri& tri = triangles[idx];
		Vec3f ptA_w = points[tri.a].diff;
		Vec3f ptB_w = points[tri.b].diff;
		Vec3f ptC_w = points[tri.c].diff;
		Vec3f lambdas = EPABarycentricCoordinates(ptA_w, ptB_w, ptC_w, Vec3f(0.0f));

		//LOG(ToString(ptA_w).GetCStr());
		//LOG(ToString(ptB_w).GetCStr());
		//LOG(ToString(ptC_w).GetCStr());
		//LOG(ToString(lambdas).GetCStr());

		// Get the point on shape A
		Vec3f ptA_a = points[tri.a].ptA;
		Vec3f ptB_a = points[tri.b].ptA;
		Vec3f ptC_a = points[tri.c].ptA;
		*ptOnA = ptA_a * lambdas[0] + ptB_a * lambdas[1] + ptC_a * lambdas[2];

		// Get the point on shape B
		Vec3f ptA_b = points[tri.a].ptB;
		Vec3f ptB_b = points[tri.b].ptB;
		Vec3f ptC_b = points[tri.c].ptB;
		*ptOnB = ptA_b * lambdas[0] + ptB_b * lambdas[1] + ptC_b * lambdas[2];

		// Return the penetration distance
		Vec3f delta = *ptOnB - *ptOnA;
		return Mag(delta);
	}

	bool GJKIntersection(RigidBody* bodyA, RigidBody* bodyB, const float bias, Vec3f* ptOnA, Vec3f* ptOnB)
	{
		//PROFILE_FUNCTION();
		const Vec3f origin = Vec3f(0, 0, 0);

		//DEBUGDrawOBB(GetEntityCollider(bodyA, 0));
		//DEBUGDrawOBB(GetEntityCollider(bodyB, 0));

		if (GJKIntersection(bodyA, bodyB))
		{
			int a = 2;
		}

		int32 numPts = 1;
		GJKPoint simplexPoints[4] = {};
		simplexPoints[0] = GetSupportPoint(bodyA, bodyB, Vec3f(1, 1, 1), 0.0f);

		real32 closestDist = 1e10f;
		bool doesContainOrigin = false;
		Vec3 newDir = simplexPoints[0].diff * -1.0f;
		do
		{
			// @NOTE: Get the new support point
			GJKPoint currentPoint = GetSupportPoint(bodyA, bodyB, newDir, 0.0f);

			// @NOTE: If the new point is the same as a previous point, then we can't expand any further
			if (HasPoint(simplexPoints, currentPoint))
			{
				break;
			}

			simplexPoints[numPts] = currentPoint;
			numPts++;

			// @NOTE: If this new point hasn't moved passed the origin, then the
			// @NOTE: origin cannot be in the set. And therefore there is no collision.
			if (Dot(newDir, (currentPoint.diff - origin)) < 0.0f)
			{
				break;
			}

			// @NOTE: Now project the origin onto the simplex
			CheckSimplexResult check = CheckSimplex(simplexPoints, numPts);
			if (check.intersect)
			{
				doesContainOrigin = check.intersect;
				break;
			}

			newDir = check.newDir;


			// @NOTE: Check that the new projection of the origin onto the simplex is closer than the previous
			real32 dist = MagSqrd(newDir);
			if (dist >= closestDist)
			{
				break;
			}
			closestDist = dist;

			// @NOTE: Use the lambdas that support the new search direction, and invalidate any points that don't support it
			numPts = SortValidPointsAndLambdas(simplexPoints, &check.lambdas);

			doesContainOrigin = (4 == numPts);

		} while (!doesContainOrigin);

		if (!doesContainOrigin) {
			return false;
		}


		// @NOTE: Check that we have a 3-simplex (EPA expects a tetrahedron)
		if (1 == numPts) {
			Vec3f searchDir = simplexPoints[0].diff * -1.0f;
			GJKPoint newPt = GetSupportPoint(bodyA, bodyB, searchDir, 0.0f);
			simplexPoints[numPts] = newPt;
			numPts++;
		}
		if (2 == numPts) {
			Vec3f ab = simplexPoints[1].diff - simplexPoints[0].diff;

			Vec3f u, v;
			GetOrtho(ab, &u, &v);

			Vec3f newDir = u;
			GJKPoint newPt = GetSupportPoint(bodyA, bodyB, newDir, 0.0f);
			simplexPoints[numPts] = newPt;
			numPts++;
		}
		if (3 == numPts) {
			Vec3f ab = simplexPoints[1].diff - simplexPoints[0].diff;
			Vec3f ac = simplexPoints[2].diff - simplexPoints[0].diff;
			Vec3f norm = Cross(ab, ac);

			Vec3f newDir = norm;
			GJKPoint newPt = GetSupportPoint(bodyA, bodyB, newDir, 0.0f);
			simplexPoints[numPts] = newPt;
			numPts++;
		}

		LOG(ToString(simplexPoints[0].ptA).GetCStr());
		LOG(ToString(simplexPoints[0].ptB).GetCStr());

		LOG(ToString(simplexPoints[1].ptA).GetCStr());
		LOG(ToString(simplexPoints[2].ptA).GetCStr());
		LOG(ToString(simplexPoints[3].ptA).GetCStr());

		// @NOTE: Get the center point of the simplex
		Vec3f simplexCenter = Vec3f(0, 0, 0);
		for (int32 i = 0; i < 4; i++) {
			simplexCenter += simplexPoints[i].diff;
		}
		simplexCenter = simplexCenter * 0.25f;

		// @NOTE: Now expand the simplex by the bias amount for accuracy reasons
		for (int32 i = 0; i < numPts; i++) {
			GJKPoint& pt = simplexPoints[i];

			Vec3f dir = Normalize(pt.diff - simplexCenter);

			pt.ptA += dir * bias;
			pt.ptB -= dir * bias;
			pt.diff = pt.ptA - pt.ptB;
		}

		EPAExpand(bodyA, bodyB, bias, simplexPoints, ptOnA, ptOnB);

		return true;
	}

}