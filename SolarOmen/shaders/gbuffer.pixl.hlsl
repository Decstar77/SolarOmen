
#include "PixelShaderHeader.hlsli"

struct VSInput
{
	float3 worldPos : WorldPos;
	float3 worldNormal : Normal;
	float2 uvs : TexCord;
};

struct PSOutput
{
	float4 position : SV_Target0;
	float4 normal : SV_Target1;
	float4 albedo : SV_Target2;
};

PSOutput main(VSInput vsInput)
{
	PSOutput output;

	output.position = float4(vsInput.worldPos, 1);
	output.normal = float4(normalize(vsInput.worldNormal), 1);
	//output.albedo = texture0.Sample(linearSampler, vsInput.uvs).rgb;
	output.albedo = float4(pow(texture0.Sample(linearSampler, vsInput.uvs).rgb, float3(2.2, 2.2, 2.2)), 1.0f);


	return output;
}
