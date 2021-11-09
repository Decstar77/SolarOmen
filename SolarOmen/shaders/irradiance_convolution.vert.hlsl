#include "VertexShaderHeader.hlsli"


struct VSOutput
{
	float3 localPos: WorldPos;
	float2 uv : TexCord;
	float3 normal : Normal;
	float4 pos : SV_POSITION;
};

VSOutput main(float3 pos : Position, float3 normal : Normal, float2 uv : TexCord)
{
	VSOutput output;
	output.uv = float2(0, 0);
	output.normal = normal;
	output.localPos = pos;
	output.pos = mul(float4(pos, 1.0f), mul(view, persp));

	return output;
}