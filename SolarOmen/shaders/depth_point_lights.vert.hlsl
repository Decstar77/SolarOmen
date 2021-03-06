cbuffer VSInput  : register(b0)
{
	matrix mvp;
	matrix persp;
	matrix view;
	matrix model;
	matrix invModel;
};

struct VSOutput
{
	float3 worldPos : WorldPos;
	float4 pos : SV_POSITION;
};

VSOutput main(float3 pos : Position, float3 normal : Normal, float2 txc : TexCord)
{
	VSOutput output;
	output.pos = mul(float4(pos, 1.0f), mvp);
	output.worldPos = mul(float4(pos, 1.0f), model).xyz;

	return output;
}