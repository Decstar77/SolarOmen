#include "PixelShaderHeader.hlsli"
struct VSInput
{
	float3 localPos : WorldPos;
	float2 uv : TexCord;
	float3 normal : Normal;
};

static const float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v)
{
	float2 uv = float2(atan2(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

float4 main(VSInput vsInput) : SV_TARGET
{
	float2 uv = SampleSphericalMap(-normalize(vsInput.localPos));
	float3 color = texture0.Sample(linearSampler, uv).rgb;

	return float4(color, 1.0);
	//return float4(vsInput.localPos, 1.0);
	//return float4(0.0, 1.0, 1.0, 1.0);
}