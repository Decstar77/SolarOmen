#include "VertexShaderHeader.hlsli"

struct VSOutput
{
	float4 pos : SV_POSITION;
};

VSOutput main(float3 pos : Position)
{
	VSOutput output;

	matrix transform = mul(view, persp);
	output.pos = mul(float4(pos, 1.0f), transform);
	return output;
}

