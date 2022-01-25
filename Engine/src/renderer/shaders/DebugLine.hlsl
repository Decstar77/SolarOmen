struct VS_INPUT
{
	float3 pos : POSITION;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
};

#include "VertexHeader.hlsli"

VS_OUTPUT VSmain(VS_INPUT input)
{
	matrix transform = mul(view, persp);

	VS_OUTPUT output;
	output.pos = mul(float4(input.pos, 1.0f), transform);

	return output;
}
#include "PixelHeader.hlsli"

#define MAX_DIRECTIONAL_LIGHT_COUNT 2
#define MAX_SPOT_LIGHT_COUNT 8
#define MAX_POINT_LIGHT_COUNT 16


float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
	return float4(0.2f, 0.8f, 0.2f, 1.0f);
}