
#include "PixelShaderHeader.hlsli"


struct VSInput
{
	float3 worldPos : WorldPos;
	float3 worldNormal : Normal;
	float2 uvs : TexCord;
	float3 colour :Colours;
};

struct PSOutput
{
	float4 colour : SV_Target0;
};

PSOutput main(VSInput vsInput)
{
	PSOutput output;

	output.colour = float4(vsInput.colour, 1);

	return output;
}
