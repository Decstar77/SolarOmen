#include "VertexShaderHeader.hlsli"

struct VSOutput
{
	float3 worldPos : WorldPos;
	float2 uvs : TexCord;
	float3 normal : Normal;
	float4 colour : Colour;
	float4 pos : SV_POSITION;
};

VSOutput main(float3 pos : Position, float3 normal : Normal, float2 txc : TexCord, float4 colour : Colour)
{
	VSOutput output;
	output.pos = mul(float4(pos, 1.0f), mvp);
	output.uvs = txc;
	output.worldPos = mul(float4(pos, 1.0f), model).xyz;
	output.colour = colour;

	matrix nm = transpose(invM);
	float4 tt = mul(float4(normal, 0.0f), nm);
	output.normal = (float3)tt;

	return output;
}