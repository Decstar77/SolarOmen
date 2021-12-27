#define MAX_DIRECTIONAL_LIGHT_COUNT 2
#define MAX_SPOT_LIGHT_COUNT 8
#define MAX_POINT_LIGHT_COUNT 16

struct DirectionalLight
{
	float3 colour;
	float3 direction;
};

struct PointLight
{
	float3 colour;
	float3 position;
};

struct SpotLight
{
	float3 colour;
	float3 direction;
	float3 position;
	float inner;
	float outter;
};

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

cbuffer LightingConstants : register(b1)
{
	float pointLightNearPlane;
	float pointLightFarPlane;
	float pointLightShadowAtlasDelta;
	float bloomThreshHold;
	// End 
	float bloomSoftness;
	float pad1;
	float pad2;
	float pad3;
}

cbuffer TransientData : register(b2)
{
	struct
	{
		int t1;
		int t2;
	} iTemps;
}

cbuffer LightData : register(b3)
{
	matrix view;
	matrix lightVP[4];
	float4 cascades;
}

cbuffer UIData : register(b4)
{
	float4 uiColour;
	float4 uiSizePos;
	int4 uiUses;
};

Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
Texture2D texture2 : register(t2);
Texture2D texture3 : register(t3);
Texture2D texture4 : register(t4);

// @NOTE: Point shadows ?
TextureCube cubeTexture5:  register(t5);
// @NOTE: Shadow cascades
Texture2DArray texture6 : register(t6);
// @NOTE: UI Texture
Texture2D texture7 : register(t7);

Texture2D bloomTexture : register(t1);
Texture2D positionBuffer: register(t2);
Texture2D normalBuffer : register(t3);
Texture2D albedoBuffer: register(t4);
Texture2D shadowBuffer : register(t5);
TextureCube shadowMap1:  register(t6);

SamplerState pointSampler : register(s0);
SamplerState linearSampler : register(s1);
SamplerState triSampler: register(s2);
SamplerComparisonState shadowPCFSampler : register(s3);

#define PI 3.14159265359