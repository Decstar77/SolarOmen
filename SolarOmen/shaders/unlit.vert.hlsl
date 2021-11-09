#include "VertexShaderHeader.hlsli"


struct VSOutput
{
	float3 world_position : WorldPos;
	float2 uv : TexCord;
	float3 normal : Normal;
	float4 pos : SV_POSITION;
};

VSOutput main(float3 pos : Position, float3 normal : Normal, float2 uv : TexCord)
{
	//VSOutput output;
	//output.pos = float4(pos, 1.0f);
	//output.uv = uv;
	//output.normal = normal;
	//output.world_position = pos;

	//return output;

	VSOutput output;
	output.pos = mul(float4(pos, 1.0f), mvp);
	output.uv = uv;
	output.world_position = mul(float4(pos, 1.0f), model).xyz;
	output.normal = mul(normal, (float3x3)transpose(invM));

	return output;
}