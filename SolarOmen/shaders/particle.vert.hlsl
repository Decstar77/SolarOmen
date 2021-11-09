cbuffer VSInput  : register(b0)
{
	matrix _mvp;
	matrix persp;
	matrix view;
	matrix _model;
	matrix _invModel;
};

struct VSOutput
{
	float3 worldPos : WorldPos;
	float3 worldNormal : Normal;
	float2 uvs : TexCord;
	float3 colour :Colours;
	float4 pos : SV_POSITION;
};

VSOutput main(float3 pos : Position, float3 normal : Normal, float2 txc : TexCord, float4x4 model : Model)
{
	VSOutput output;
	output.colour = float3(model[0][3], model[1][3], model[2][3]);
	model[0][3] = 0;
	model[1][3] = 0;
	model[2][3] = 0;

	matrix mvp = mul(mul(model, view), persp);

	output.pos = mul(float4(pos, 1.0f), mvp);
	output.uvs = txc;
	output.worldPos = mul(float4(pos, 1.0f), model).xyz;

	output.worldNormal = normal;

	return output;
}