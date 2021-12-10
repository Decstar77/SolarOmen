#include "PixelShaderHeader.hlsli"
struct VSInput
{
	float3 world_position : WorldPos;
	float2 uv : TexCord;
	float3 normal : Normal;
};

float4 main(VSInput vsInput) : SV_TARGET
{
	return float4 (texture0.Sample(linearSampler, vsInput.uv).rgb, 1);
//return float4 (1,1,1, 1);
//return float4(normalize(vsInput.normal), 1.0f);
//return float4(vsInput.uv.x, vsInput.uv.y, 0.0f, 1.0f);
}