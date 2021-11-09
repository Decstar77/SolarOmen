#include "VertexShaderHeader.hlsli"


struct VSOutput
{
	float3 worldPos : WorldPos;
	float4 pos : SV_POSITION;
};

VSOutput main(float3 pos : Position, float3 normal : Normal, float2 txc : TexCord)
{
	VSOutput output;
	output.worldPos = pos;
	output.pos = mul(float4(pos, 0.0f), mul(view, persp));
	output.pos.z = output.pos.w;

	return output;
}