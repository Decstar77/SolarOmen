struct VS_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TexCord;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : TexCord;
};

#include "VertexHeader.hlsli"

VS_OUTPUT VSmain(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul(float4(input.pos, 1.0f), mvp);
	output.normal = input.normal;
	output.uv = input.uv;

	return output;
}

#include "PixelHeader.hlsli"

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
	float4 fragColour = float4(0.0f, 0.0f, 0.0f, 1.0f);
	//float3 colour = pow(texture0.Sample(linearSamplerRepeat, input.uv).rgb, float3(2.2, 2.2, 2.2));
	float3 colour = texture0.Sample(pointSamplerRepeat, input.uv).rgb;


	fragColour = float4(colour, 1.0f);
	return fragColour;
}