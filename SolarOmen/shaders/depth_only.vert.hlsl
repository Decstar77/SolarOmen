#include "VertexShaderHeader.hlsli"

float4 main(float3 pos : Position, float3 normal : Normal, float2 txc : TexCord) : SV_POSITION
{
	float4 p = mul(float4(pos, 1.0f), mvp);
	return p;
}