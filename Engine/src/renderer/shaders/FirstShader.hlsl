struct VS_INPUT
{
	float3 pos : POSITION;
	float4 color: COLOUR;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float4 color: COLOUR;
};

struct VertexBuffer
{
	float4 colorMultiplier;
};

ConstantBuffer<VertexBuffer> vertexBuffer : register(b0, space0);

VS_OUTPUT VSmain(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = float4(input.pos, 1.0f);
	output.color = input.color * vertexBuffer.colorMultiplier;
	return output;
}

float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
	// return interpolated color
	return input.color;
}