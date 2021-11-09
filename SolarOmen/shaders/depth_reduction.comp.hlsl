
Texture2D<float> depthMap : register(t0);
RWTexture2D<unorm float2> outputMap : register(u0);

cbuffer reductionConstants : register(b0)
{
	float4x4 projection;
	float4 clippingPlanes;
}

#define BlockSize 16
#define ThreadCount BlockSize * BlockSize 

groupshared float2 depthSamples[ThreadCount];

[numthreads(BlockSize, BlockSize, 1)]
void main(in uint3 GroupID : SV_GroupID, in uint3 GroupThreadID : SV_GroupThreadID,
	in uint ThreadIndex : SV_GroupIndex, in uint3 dispatchThreadID : SV_DispatchThreadID)
{
	float minDepth = 1.0f;
	float maxDepth = 0.0f;

	uint2 textureSize;
	depthMap.GetDimensions(textureSize.x, textureSize.y);

	uint2 samplePos = min(dispatchThreadID.xy, textureSize - 1);

	float depthSample = depthMap[samplePos];

	if (depthSample < 1.0f)
	{
		float nearClip = clippingPlanes.x;
		float farClip = clippingPlanes.y;
		// Convert to linear Z
		depthSample = projection._43 / (depthSample - projection._33);
		depthSample = saturate((depthSample - nearClip) / (farClip - nearClip));
		minDepth = min(minDepth, depthSample);
		maxDepth = max(maxDepth, depthSample);
	}

	depthSamples[ThreadIndex] = float2(minDepth, maxDepth);
	GroupMemoryBarrierWithGroupSync();

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