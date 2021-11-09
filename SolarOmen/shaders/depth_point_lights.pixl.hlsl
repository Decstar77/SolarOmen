#include "PixelShaderHeader.hlsli"

struct VSInput
{
	float3 worldPos: WorldPos;
};

float2 main(VSInput vsInput, uint pid : SV_PrimitiveID) : SV_TARGET
{
	float3 pos = pointLights[iTemps.t1].position;
	float dist = distance(vsInput.worldPos, pos);

	float dx = ddx(pid);
	float dy = ddy(dist);
	float m2 = dist * dist + 0.25 * (dx * dx + dy * dy);

	return float2(iTemps.t2, pid);
	//return float4(dist, dist * dist, dist, 1.0f);
	//return float4(1.0f, 0.0f, 0.0f, 1.0f);
}