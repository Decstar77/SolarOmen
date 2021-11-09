Texture2D<unorm float2> reductionMap : register(t0);
RWTexture2D<unorm float2> outputMap : register(u0);

#define BlockSize 16
#define ThreadCount BlockSize * BlockSize 

groupshared float2 depthSamples[ThreadCount];

[numthreads(BlockSize, BlockSize, 1)]
void main(in uint3 GroupID : SV_GroupID, in uint3 GroupThreadID : SV_GroupThreadID,
	in uint ThreadIndex : SV_GroupIndex, in uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint2 textureSize;
	reductionMap.GetDimensions(textureSize.x, textureSize.y);

	uint2 samplePos = min(dispatchThreadID.xy, textureSize - 1);

	float minDepth = reductionMap[samplePos].x;
	float maxDepth = reductionMap[samplePos].y;

	if (minDepth == 0.0f)
		minDepth = 1.0f;

	// Store in shared memory
	depthSamples[ThreadIndex] = float2(minDepth, maxDepth);
	GroupMemoryBarrierWithGroupSync();

	// Reduce
	[unroll]
	for (uint s = ThreadCount / 2; s > 0; s >>= 1)
	{
		if (ThreadIndex < s)
		{
			depthSamples[ThreadIndex].x = min(depthSamples[ThreadIndex].x, depthSamples[ThreadIndex + s].x);
			depthSamples[ThreadIndex].y = max(depthSamples[ThreadIndex].y, depthSamples[ThreadIndex + s].y);
		}

		GroupMemoryBarrierWithGroupSync();
	}

	if (ThreadIndex == 0)
	{
		minDepth = depthSamples[0].x;
		maxDepth = depthSamples[0].y;
		outputMap[GroupID.xy] = float2(minDepth, maxDepth);
	}
}