struct VS_INPUT
{
	float3 pos : POSITION;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
};

cbuffer ModelData : register(b0)
{
	matrix mvp;
	matrix model;
	matrix invM;
};

cbuffer ViewData : register(b1)
{
	matrix persp;
	matrix view;
	matrix screenProjection;
}

VS_OUTPUT VSmain(VS_INPUT input)
{
	matrix transform = mul(view, persp);

	VS_OUTPUT output;
	output.pos = mul(float4(input.pos, 1.0f), transform);

	return output;
}

Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
Texture2D texture2 : register(t2);
Texture2D texture3 : register(t3);
Texture2D texture4 : register(t4);
Texture2D texture5 : register(t5);
Texture2D texture6 : register(t6);
Texture2D texture7 : register(t7);
Texture2D texture8 : register(t8);
Texture2D texture9 : register(t9);
Texture2D texture10 : register(t10);

SamplerState pointSamplerRepeat			: register(s0);
SamplerState linearSamplerRepeat		: register(s1);
SamplerState triSamplerRepeat			: register(s2);
SamplerState pointSamplerClamp			: register(s3);
SamplerState linearSamplerClamp			: register(s4);
SamplerState triSamplerClamp			: register(s5);
SamplerState antiSamplerClamp			: register(s6);
SamplerComparisonState shadowPCFSampler : register(s7);

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

cbuffer LightingConstants : register(b0)
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

float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
	return float4(0.2f, 0.8f, 0.2f, 1.0f);
}