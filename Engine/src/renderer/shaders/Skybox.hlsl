struct VSInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TexCord;
};

struct VSOutput
{
	float3 worldPos : WorldPos;
	float4 pos : SV_POSITION;
};

#include "VertexHeader.hlsli"

VSOutput VSmain(VSInput input)
{
	matrix transform = mul(view, persp);

	VSOutput output;
	output.worldPos = input.pos;
	output.pos = mul(float4(input.pos, 0.0f), mul(view, persp));
	output.pos.z = output.pos.w;

	return output;
}

#include "PixelHeader.hlsli"

float4 PSmain(VSOutput vsInput) : SV_TARGET
{
	 float3 uvw = normalize(vsInput.worldPos);

	 //float3 envColor = pow(cubeTexture5.Sample(linearSampler, uvw).rgb, float3(2.2, 2.2, 2.2));
	 float3 envColor = texture10Cube.Sample(linearSamplerClamp,uvw).rgb;

	 return float4(envColor, 1.0);
}
