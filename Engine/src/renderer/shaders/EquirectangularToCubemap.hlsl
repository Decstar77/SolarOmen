
struct VSInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TexCord;
};

struct VSOutput
{
	float3 localPos: WorldPos;
	float2 uv : TexCord;
	float3 normal : Normal;
	float4 pos : SV_POSITION;
};

#include "VertexHeader.hlsli"

VSOutput VSmain(VSInput input)
{
	VSOutput output;
	output.uv = float2(0, 0);
	output.normal = input.normal;
	output.localPos = input.pos;
	output.pos = mul(float4(input.pos, 1.0f), mul(view, persp));

	return output;
}

#include "PixelHeader.hlsli"

static const float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v)
{
	float2 uv = float2(atan2(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

float4 PSmain(VSOutput vsInput) : SV_TARGET
{
	float2 uv = SampleSphericalMap(-normalize(vsInput.localPos));
	float3 color = texture0.Sample(linearSamplerClamp, uv).rgb;

	return float4(color, 1.0);
}