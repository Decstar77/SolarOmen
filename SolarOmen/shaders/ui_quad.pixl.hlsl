#include "PixelShaderHeader.hlsli"

struct VSInput
{
	float2 uv : UV;
	float4 screenUV : SV_Position;
};

float roundedBoxSDF(float2 CenterPosition, float2 Size, float Radius) {
	return length(max(abs(CenterPosition) - Size + Radius, 0.0)) - Radius;
}

float4 main(VSInput vsInput) : SV_TARGET
{
	float2 size = uiSizePos.xy;
	float2 pos = uiSizePos.zw - (size / 2);
	float radius = 1.0f;
	float2 fc = vsInput.screenUV.xy;

	float d = -roundedBoxSDF(fc - pos - (size / 2.0f), size, radius) + 1;
	float4 colour = uiColour * float4(1, 1, 1, d);

	if (uiUses.x > 0)
		  colour *= texture7.Sample(linearSampler, vsInput.uv);

	return colour;
}