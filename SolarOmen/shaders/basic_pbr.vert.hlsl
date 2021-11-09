#include "VertexShaderHeader.hlsli"

cbuffer LightingInfo : register(b1)
{
	matrix lightViewProj;
	matrix lightView;
}

struct VSOutput
{
	float3 world_position : WorldPos;
	float2 texture_coords : TexCord;
	float3 normal : Normal;
	float4 pos : SV_POSITION;
};

VSOutput main(float3 pos : Position, float3 normal : Normal, float2 txc : TexCord)
{
	VSOutput output;
	output.pos = mul(float4(pos, 1.0f), mvp);
	output.texture_coords = txc;
	output.world_position = mul(float4(pos, 1.0f), model).xyz;
	output.normal = mul(normal, (float3x3)transpose(invM));

	//output.lightViewProj = mul(float4(output.world_position, 1.0), lightViewProj);
	//output.lightViewProj = float4(0, 0, 0, 0);
	//output.lightView = float4(0, 0, 0, 0);
	//output.lightView = mul(float4(output.world_position, 1.0), lightView);
	// output.normal = normal;

	return output;
}

