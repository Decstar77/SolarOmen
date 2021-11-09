#include "PixelShaderHeader.hlsli"

struct VSInput
{
	float2 uv: TexCord;
};

float PCFShadow(float2 uv)
{
	uint width;
	uint height;
	uint levels;
	shadowBuffer.GetDimensions(0, width, height, levels);

	float2 texelSize = 1.0f / float2((float)width, (float)height);

	float s0 = shadowBuffer.Sample(pointSampler, uv).r;
	float s1 = shadowBuffer.Sample(pointSampler, uv + float2(texelSize.x, 0)).r;
	float s2 = shadowBuffer.Sample(pointSampler, uv - float2(texelSize.x, 0)).r;
	float s3 = shadowBuffer.Sample(pointSampler, uv + float2(0, texelSize.y)).r;
	float s4 = shadowBuffer.Sample(pointSampler, uv - float2(0, texelSize.y)).r;

	return (s0 + s1 + s2 + s3 + s4) / 5.0f;
}

float SoftShadow(float2 uv, float zPos)
{
	float z = zPos / pointLightFarPlane;

	float occ = shadowBuffer.Sample(linearSampler, uv).r;
	float lightSize = 1.0f;
	float sampleSize = ((1.0 - occ) * lightSize) / z;


	uint width;
	uint height;
	uint levels;
	shadowBuffer.GetDimensions(0, width, height, levels);

	float2 texelSize = float2(1.0f / width, 1.0f / height);

	float maxValue = -1;
	float minDist = 1;

	int s = int(sampleSize);
	for (int x = -s; x <= s; x++)
	{
		for (int y = -s; y <= s; y++)
		{
			float2 offset = float2(x, y) * texelSize;
			float comp = shadowBuffer.Sample(pointSampler, uv + offset).r;

			float compDist = dot(float2(x, y), float2(x, y));

			if (comp > maxValue && compDist < minDist)
			{
				minDist = compDist;
			}
		}
	}

	float dMax = sampleSize * sampleSize + sampleSize * sampleSize;

	return 1;
}

float4 main(VSInput vsInput) : SV_TARGET
{
	float3 colour = albedoBuffer.Sample(pointSampler, vsInput.uv).rgb;
	float3 worldPos = positionBuffer.Sample(pointSampler, vsInput.uv).xyz;
	float3 worldNormal = normalBuffer.Sample(pointSampler, vsInput.uv).xyz;

	float shadow = shadowBuffer.Sample(linearSampler, vsInput.uv).r;
	//float shadow = PCFShadow(vsInput.uv);
	//float shadow = SoftShadow(vsInput.uv, worldPos.z);

	float4 fragColour = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float3 ambient = 0.01 * colour;

	for (int pointLightIndex = 0; pointLightIndex < lightCounts.pointLightCount; pointLightIndex++)
	{
		float3 lightPos = pointLights[pointLightIndex].position;
		float3 lightColour = pointLights[pointLightIndex].colour;

		float dist = distance(lightPos, worldPos);
		float attenuation = 1.0 / (1.0f + 0.09f * dist + 0.032f * (dist * dist));
		//float attenuation = 1.0;

		float3 lightDir = normalize(lightPos - worldPos);
		float3 normal = normalize(worldNormal);
		float diff = max(dot(lightDir, normal), 0.0);
		float3 diffuse = diff * lightColour * colour * (1 - shadow) * attenuation;

		float3 viewDir = normalize(viewPos - worldPos);
		float3 reflectDir = reflect(-lightDir, normal);

		float3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

		float3 specular = lightColour * spec * 0.15f * (1 - shadow) * attenuation;

		fragColour += float4(diffuse + specular, 1.0);
		shadow = 0;
	}

	fragColour += float4(ambient, 1.0);

	return fragColour;
}