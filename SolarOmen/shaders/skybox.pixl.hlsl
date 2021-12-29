#include "PixelShaderHeader.hlsli"

struct VSInput
{
	float3 worldPos : WorldPos;
};

float4 main(VSInput vsInput) : SV_TARGET
{
	 float3 uvw = normalize(vsInput.worldPos);

	 //float3 envColor = pow(cubeTexture5.Sample(linearSampler, uvw).rgb, float3(2.2, 2.2, 2.2));

	 float3 envColor = texture10Cube.Sample(linearSampler,uvw).rgb;
	 //envColor = envColor / (envColor + float3(1.0, 1.0, 1.0));
	 //envColor = pow(envColor, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
	 return float4(envColor, 1.0);
	 //return float4(.5f, 1.0f, 0.6f, 1.0f);
}