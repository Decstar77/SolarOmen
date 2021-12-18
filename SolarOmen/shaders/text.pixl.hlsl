#include "PixelShaderHeader.hlsli"

struct VSInput
{
	float2 uv : UV;
};

float4 main(VSInput vsInput) : SV_TARGET
{
	float c = texture0.Sample(linearSampler, vsInput.uv).r;
return float4(c, c, c, c);
//return float4(1.0f, 1.0f, 1.0f, 1.0f);
}