#include "VertexShaderHeader.hlsli"

struct VSOutput
{
	float2 uv : UV;
	float4 pos : SV_POSITION;
};


VSOutput main(float4 pos : POSITION)
{
	VSOutput output;
	output.uv = pos.zw;
	output.pos = mul(float4(pos.x, pos.y, 0.0, 1.0), screenProjection);

	return output;
}