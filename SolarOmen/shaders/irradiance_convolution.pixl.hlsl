#include "PixelShaderHeader.hlsli"
#include "MatrixUtilHeader.hlsli"

struct VSInput
{
	float3 localPos : WorldPos;
	float2 uv : TexCord;
	float3 normal : Normal;
};




float4 main(VSInput vsInput) : SV_TARGET
{

	float3 N = normalize(vsInput.localPos);
	float3 up = float3 (0.0, 1.0, 0.0);
	float3 right = normalize(cross(up, N));
	up = normalize(cross(N, right));

	float3x3 M = CreateBasis(vsInput.localPos, float3(0.0, 1.0, 0.0));

	//M[0] = right;
	//M[1] = up;
	//M[2] = N;

	float sampleDelta = 0.025;
	float nrSamples = 0.0;

	float3 irradiance = float3(0,0,0);
	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

			//float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
			float3 sampleVec = mul(tangentSample, M);

			irradiance += cubeTexture5.Sample(linearSampler, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}

	irradiance = PI * irradiance * (1.0 / float(nrSamples));

	return float4(irradiance, 1.0);
	//return float4(vsInput.localPos, 1.0);
	//return float4(0.0, 1.0, 1.0, 1.0);
}