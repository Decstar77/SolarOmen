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
	float3 worldNormal : Normal;
	float2 uvs : TexCord;
	float4 pos : SV_POSITION;
};

VSOutput main(float3 pos : Position, float3 normal : Normal, float2 txc : TexCord)
{
	VSOutput output;
	output.pos = mul(float4(pos, 1.0f), mvp);
	output.uvs = txc;
	output.worldPos = mul(float4(pos, 1.0f), model).xyz;

	matrix nm = transpose(invModel);
	float4 tt = mul(float4(normal, 0.0f), nm);
	output.worldNormal = (float3)tt;

	return output;
}