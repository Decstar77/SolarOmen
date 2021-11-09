#define MAX_DIRECTIONAL_LIGHT_COUNT 2
#define MAX_SPOT_LIGHT_COUNT 8
#define MAX_POINT_LIGHT_COUNT 16

cbuffer LightingInfo : register(b0)
{
	float3 viewPos;

	struct
	{
		int dirLightCount;
		int spotLightCount;
		int pointLightCount;
		int pad1;
	} lightCounts;

	struct {
		float3 direction;
		float3 colour;
	} directionalLights[MAX_DIRECTIONAL_LIGHT_COUNT];

	struct {
		float4 position;		// @NOTE: z = inner cuttoff
		float4 direction;		// @NOTE: y = outter cuttoff
		float3 colour;
	} spotLights[MAX_SPOT_LIGHT_COUNT];

	struct {
		float3 position;
		float3 colour;
	} pointLights[MAX_POINT_LIGHT_COUNT];
}

cbuffer TransientData : register(b2)
{
	struct
	{
		int t1;
		int t2;
	} iTemps;
}

struct PACKEDNODE
{
	float4 a;
	float4 b;
	float4 c;
	float4 d;
};

StructuredBuffer<PACKEDNODE> scene : register(t0);

Texture2D<float4> positionBuffer : register(t2);
Texture2D<float4> normalBuffer: register(t3);
Texture2D<float4> albedoBuffer: register(t4);

RWTexture2D<float4> shadowBuffer : register(u0);

struct AABB
{
	float3 min;
	float3 max;
};

struct Ray
{
	float3 origin;
	float3 dir;
};

#define EPSILON 0.001
bool RaycastAABB(Ray r, float3 bmin, float3 bmax, out float dist)
{
	float3 invDir = 1.0f / r.dir;

	float3 t1 = (bmin - r.origin) * (invDir);
	float3 t2 = (bmax - r.origin) * (invDir);

	float3 tmin3 = min(t1, t2);
	float3 tmax3 = max(t1, t2);

	float tmin = max(max(tmin3.x, tmin3.y), tmin3.z);
	float tmax = min(min(tmax3.x, tmax3.y), tmax3.z);

	dist = tmin;

	return tmax > 0 && tmax > tmin;
}

// @Credit: https://gist.github.com/mattatz/86fff4b32d198d0928d0fa4ff32cf6fa
float3x3 QuatToMat3(float4 quat)
{
	float3x3 m = float3x3(float3(0, 0, 0), float3(0, 0, 0), float3(0, 0, 0));

	float x = quat.x, y = quat.y, z = quat.z, w = quat.w;
	float x2 = x + x, y2 = y + y, z2 = z + z;
	float xx = x * x2, xy = x * y2, xz = x * z2;
	float yy = y * y2, yz = y * z2, zz = z * z2;
	float wx = w * x2, wy = w * y2, wz = w * z2;

	m[0][0] = 1.0 - (yy + zz);
	m[0][1] = xy - wz;
	m[0][2] = xz + wy;

	m[1][0] = xy + wz;
	m[1][1] = 1.0 - (xx + zz);
	m[1][2] = yz - wx;

	m[2][0] = xz - wy;
	m[2][1] = yz + wx;
	m[2][2] = 1.0 - (xx + yy);

	return m;
}

bool2 LessThan(float2 a, float2 b)
{
	return bool2(a.x < b.x, a.y < b.y);
}

// @Credit: http://www.jcgt.org/published/0007/03/04/paper-lowres.pdf
bool RaycastOBB(Ray ray, float3 center, float3 radius, float4 quatRot, out float dist)
{
	float3x3 rotation = QuatToMat3(quatRot);
	ray.origin = mul(ray.origin - center, rotation);
	ray.dir = mul(ray.dir, rotation);

	float3 sgn = -sign(ray.dir);

	float3 distanceToPlane = radius * sgn - ray.origin;
	distanceToPlane /= ray.dir;

#   define TEST(U, VW)\
         /* Is there a hit on this axis in front of the origin? Use multiplication instead of && for a small speedup */\
         (distanceToPlane.U >= 0.0) && \
         /* Is that hit within the face of the box? */\
         all(LessThan(abs(ray.origin.VW + ray.dir.VW * distanceToPlane.U), radius.VW))

	bool3 test = bool3(TEST(x, yz), TEST(y, zx), TEST(z, xy));
# undef TEST

	sgn = test.x ? float3(sgn.x, 0.0, 0.0) : (test.y ? float3(0.0, sgn.y, 0.0) : float3(0.0, 0.0, test.z ? sgn.z : 0.0));

	dist = (sgn.x != 0.0) ? distanceToPlane.x : ((sgn.y != 0.0) ? distanceToPlane.y : distanceToPlane.z);

	return (sgn.x != 0) || (sgn.y != 0) || (sgn.z != 0);
}

float RaytraceShadow(float3 worldP, float3 worldN, float3 lightP)
{
	float shadow = 0;
	float d = distance(lightP, worldP);
	float3 lightDir = (lightP - worldP) / d;

	if (dot(lightDir, worldN) > 0)
	{

#if 0
		for (int boxIndex = 0; boxIndex < iTemps.t1; boxIndex++)
		{
			AABBPACKED packed = scene[boxIndex];

			AABB box;
			box.min = packed.min.xyz;
			box.max = packed.max.xyz;

			Ray ray;
			// @NOTE: Move the origin just off the surface of 
			//		: the object to avoid self intersection with ray

			ray.origin = worldP + lightDir * EPSILON;
			ray.dir = lightDir;

			float dd;
			bool hit = RaycastAABB(ray, box, dd);

			if (hit)
			{
				if (dd < d)
				{
					shadow = 1;
					break;
				}
			}
		}
#else
		Ray ray;
		ray.origin = worldP + worldN * EPSILON;
		ray.dir = lightDir;

		//uint index = 0;
		//while (index != 0xFFFFFFFF)
		int index = 0;
		while (index >= 0)
		{
			PACKEDNODE node = scene[index];

			if (node.c.w == 0.0)
			{
				float dd = 0;
				index = RaycastAABB(ray, node.a.xyz, node.b.xyz, dd) ? int(node.a.w) : int(node.b.w);
			}
			else
			{
				float dist;
				//bool hit = RaycastAABB(ray, node.a.xyz, node.b.xyz, dist);
				bool hit = RaycastOBB(ray, node.a.xyz, node.b.xyz, node.d, dist);

				if (hit && dist < d)
				{
					shadow = 1;
					index = -1;
				}
				else
				{
					index = int(node.b.w);
				}
			}
		}
#endif
	}

	return shadow;
}

[numthreads(32, 32, 1)]
void main(uint3 groupId : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint2 uv = dispatchThreadID.xy;

	float4 storedP = positionBuffer[uv];
	float4 storedN = normalBuffer[uv];
	if (storedP.a > 0.0)
	{
		float s = RaytraceShadow(storedP.xyz, storedN.xyz, pointLights[0].position);
		shadowBuffer[uv] = float4(s, s, s, 1.0f);
	}
	else
	{
		shadowBuffer[uv] = float4(0.0f, 0.0, 0.0, 1.0f);
	}
}