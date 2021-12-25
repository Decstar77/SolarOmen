#include "VertexShaderHeader.hlsli"

struct VSOutput
{
	float2 uv : UV;
	float4 pos : SV_POSITION;
};


VSOutput main(float3 pos : Position, float3 normal : Normal, float2 txc : TexCord)
{
	VSOutput output;
	output.uv = txc;
	output.pos = float4(pos.x, pos.y, pos.z, 1.0);

	return output;
}