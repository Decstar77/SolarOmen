
#include "PixelShaderHeader.hlsli"

float4 main(float2 uv: TexCord) : SV_TARGET
{
	//const float threshHold = 1.0f;
	//const float softness = 0.5f;

	const float threshHold = bloomThreshHold;
	const float softness = bloomSoftness;

	float3 c = texture0.Sample(pointSampler, uv).rgb;

#if 1
	float brightness = max(c.r, max(c.g, c.b));
	float knee = softness * threshHold;

	float soft = brightness - threshHold + knee;
	soft = clamp(soft, 0, 2.0f * knee);
	soft = soft * soft / (4 * knee + 0.00001f);

	float contribution = max(soft, brightness - threshHold);
	contribution /= max(brightness, 0.00001);

	return float4(c * contribution, 1.0);
#else
	float brightness = dot(c, float3(0.2126, 0.7152, 0.0722));

	if (brightness > 1.0f)
		return float4(c, 1);
	else
		return float4(0,0,0, 1);
#endif

}