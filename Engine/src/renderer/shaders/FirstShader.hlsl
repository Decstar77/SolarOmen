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
	float4x4 mvp;
};

struct ColourBuffer
{
	float4 colour;
};

ConstantBuffer<VertexBuffer> vertexBuffer : register(b0, space0);
ConstantBuffer<ColourBuffer> colourBuffer : register(b1, space0);

VS_OUTPUT VSmain(VS_INPUT input)
{
	VS_OUTPUT output;
	output.pos = mul(float4(input.pos, 1.0f), vertexBuffer.mvp);
	output.color = colourBuffer.colour;
	return output;
}

float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
	// return interpolated color
	return input.color;
}