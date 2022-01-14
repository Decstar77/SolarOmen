struct VS_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : UV;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : UV;
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
	output.normal = input.normal;
	output.uv = input.uv;

	return output;
}

Texture2D t0 : register(t0);
SamplerState s0 : register(s0);

float4 PSmain(VS_OUTPUT input) : SV_TARGET
{
	// return interpolated color
	return float4(input.uv.x, input.uv.y, 0.0, 1.0);
}