
#include "PixelShaderHeader.hlsli"


static const float gamma = 2.2;
static const float exposure = 1.0f;

// Filmic Tonemapping Operators http://filmicworlds.com/blog/filmic-tonemapping-operators/
float3 tonemapFilmic(float3 x) {
	float3 X = max(float3(0, 0, 0), x - float3(0.004, 0.004, 0.004));
	float3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
	return pow(result, float3(2.2, 2.2, 2.2));
}

float3 tonemapReinhard(float3 x)
{
	float3 result = float3(1, 1, 1) - exp(-x * exposure);
	return result;
}


float4 main(float2 texture_coords : TexCord) : SV_TARGET
{
	float3 color = texture0.Sample(pointSampler, texture_coords).rgb;
	color += bloomTexture.Sample(pointSampler, texture_coords).rgb;
	// HDR tonemapping
	color = tonemapReinhard(color);

	// gamma correct
	color = pow(color, float3(1.0 / gamma, 1.0 / gamma, 1.0 / gamma));

	//return float4 (color, 1);
	return float4(texture_coords.xy, 0.0f, 1.0f);
}