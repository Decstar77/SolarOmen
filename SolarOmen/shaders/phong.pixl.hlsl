#include "PixelShaderHeader.hlsli"

struct VSInput
{
	float3 worldPos: WorldPos;
	float2 uvs : TexCord;
	float3 normal : Normal;
};

float CalculateShadow(float3 worldP, float3 lightP, float3 normal, int lightIndex)
{
	float shadow = 0.0;
	float3 toLight = worldP - lightP;
	float currentDepth = length(toLight);


#if 1
	return  shadowMap1.Sample(pointSampler, toLight).x < currentDepth;
	float3 uvw = worldP - lightP;
	//float bias = 0.1;
	float bias = max(0.001 * (1.0 - dot(normal, normalize(toLight))), 0.0001);
	float offset = 0.001f;

	shadow = 0;

	//shadow += 1 - shadowMap1.SampleCmpLevelZero(pcfSampler, uvw, (currentDepth)-bias);
	//shadow += 1 - shadowMap1.SampleCmpLevelZero(pcfSampler, uvw + float3(offset, 0, 0), (currentDepth)-bias);
	//shadow += 1 - shadowMap1.SampleCmpLevelZero(pcfSampler, uvw - float3(offset, 0, 0), (currentDepth)-bias);
	//shadow += 1 - shadowMap1.SampleCmpLevelZero(pcfSampler, uvw + float3(0, offset, 0), (currentDepth)-bias);
	//shadow += 1 - shadowMap1.SampleCmpLevelZero(pcfSampler, uvw - float3(0, offset, 0), (currentDepth)-bias);
	//shadow += 1 - shadowMap1.SampleCmpLevelZero(pcfSampler, uvw + float3(0, 0, offset), (currentDepth)-bias);
	//shadow += 1 - shadowMap1.SampleCmpLevelZero(pcfSampler, uvw - float3(0, 0, offset), (currentDepth)-bias);

	shadow /= 7;
#elif 1
	float3 uvw = worldP - lightP;
	//float bias = max(0.01 * (1.0 - dot(normal, normalize(toLight))), 0.001);
	float bias = 0.001;
	shadow += 1 - shadowMap1.SampleCmpLevelZero(pcfSampler, uvw, currentDepth - bias);

#else
	float bias = 0.01f;
	float2 moments = shadowMap1.Sample(linearSampler, toLight).xy * pointLightFarPlane;

	float compare = currentDepth - bias;

	//float p = (compare > moments.x) ? 1.0f : 0.0f;
	float p = step(moments.x + bias, currentDepth);

	float variance = max(moments.y - moments.x - moments.x, 0.0001f);

	float d = compare - moments.x;
	// @NOTE: Reduces light bleeding
	float pMax = LinearStep(0.01, 1.0, variance / (variance + d * d));

	return min(max(p, pMax), 1.0f);
#endif
	return shadow;
}

float4 main(VSInput vsInput) : SV_TARGET
{
	float4 fragColour = float4(0.0f, 0.0f, 0.0f, 1.0f);

	float3 colour = pow(texture0.Sample(linearSampler, vsInput.uvs).rgb, float3(2.2, 2.2, 2.2));

	//if (lightCounts.z > 0)
	//for (int pointLightIndex = 0; pointLightIndex < lightCounts.pointLightCount; pointLightIndex++)
	{
		//float3 lightPos = pointLights[pointLightIndex].position;
		//float3 lightColour = pointLights[pointLightIndex].colour;

		float3 normal = normalize(vsInput.normal);

		//float dist = distance(lightPos, vsInput.worldPos);
		//float attenuation = 1.0 / (1.0f + 0.09f * dist + 0.032f * (dist * dist));
		float attenuation = 1.0;

		float shadow = 0;
		//if (pointLightIndex == 0)
		//	shadow = CalculateShadow(vsInput.worldPos, lightPos, normal, pointLightIndex);

		float3 lightColour = float3(1,1, 1) * 1.1;
		float3 lightDir = normalize(float3(1,1,1));// normalize(lightPos - vsInput.worldPos);

		float diff = max(dot(lightDir, normal), 0.0);
		float3 diffuse = diff * lightColour * colour * (1 - shadow) * attenuation;

		float3 viewDir = normalize(viewPos - vsInput.worldPos);
		float3 reflectDir = reflect(-lightDir, normal);

		float3 halfwayDir = normalize(lightDir + viewDir);
		float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

		float3 specular = lightColour * spec * 0.15f * (1 - shadow) * attenuation;

		fragColour += float4(diffuse + specular, 1.0);
	}

	float3 ambient = texture11Cube.Sample(linearSampler, vsInput.normal).rgb * colour;
	fragColour += float4(ambient, 1.0);

	return fragColour;
}