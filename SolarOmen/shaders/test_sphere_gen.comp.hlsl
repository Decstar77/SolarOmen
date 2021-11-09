RWTexture2D<float4> output : register(u0);

[numthreads(512, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint width;
	uint height;
	output.GetDimensions(width, height);

	float2 p = float2(dispatchThreadID.x, dispatchThreadID.y);
	float2 c = float2(width / 2, height / 2);

	float d = distance(p, c);
	if (d < 10)
	{
		output[dispatchThreadID.xy] = float4(1, 0, 0, 1);
	}
	else
	{
		output[dispatchThreadID.xy] = float4(0, 0, 0, 1);
	}
}