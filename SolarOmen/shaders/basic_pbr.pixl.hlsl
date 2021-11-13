#include "PixelShaderHeader.hlsli"

struct VSInput
{
	float3 world_position : WorldPos;
	float2 texture_coords : TexCord;
	float3 normal : Normal;
};

struct Material
{
	float3 albedo;
	float3 F0;
	float roughness;
	float metallic;
};

struct PixelInfo
{
	float3 normal;
	float3 worldPos;
	float3 toView;
	float4 lightPos;
};

static bool USE_DEBUG = false;
static float4 DEBUG_COLOUR = float4(0, 0, 0, 1);

float3 HackGetNormalFromMap(Texture2D normalMap, float3 worldPos, float3 normal, float2 uv)
{
	float3 tangentNormal = normalMap.Sample(linearSampler, uv).xyz * 2.0 - 1.0;;

	float3 Q1 = ddx(worldPos);
	float3 Q2 = ddy(worldPos);
	float2 st1 = ddx(uv);
	float2 st2 = ddy(uv);

	float3 N = normalize(normal);
	float3 T = normalize(Q1 * st2.r - Q2 * st1.r);
	float3 B = -normalize(cross(N, T));
	float3x3 TBN = float3x3(T, B, N);

	return normalize(mul(tangentNormal, TBN));
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / max(denom, 0.0000001); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
// ----------------------------------------------------------------------------
float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
// ----------------------------------------------------------------------------
float3 CalculateEnvirmonmentLighting(PixelInfo info, Material materail)
{

}
// ----------------------------------------------------------------------------
float ShadowCalculation(float3 worldPos, float3 worldNormal, float3 lightDir)
{
#if 1
	float zDepth = mul(float4(worldPos, 1.0), view).z;

	int layer = 3;
	for (int i = 0; i < 4; i++)
	{
		if (zDepth < cascades[i])
		{
			layer = i;
			break;
		}
	}

#if 0
	USE_DEBUG = true;
	if (layer == 0)
	{
		DEBUG_COLOUR = float4(1, 1, 0, 1);
	}
	else if (layer == 1)
	{
		DEBUG_COLOUR = float4(0, 1, 0, 1);
	}
	else if (layer == 2)
	{
		DEBUG_COLOUR = float4(0, 1, 1, 1);
	}
	else
	{
		DEBUG_COLOUR = float4(0, 0, 1, 1);
	}
#endif

	float4 lightPos = mul(float4(worldPos, 1.0), lightVP[layer]);

	// @NOTE: this divide by w is meaninless when useing ortho
	float3 projCoords = lightPos.xyz / lightPos.w;

	// @NOTE: No change in ZZZZZZZZ because in d3d depth is [0, 1] and in gl [-1, 1] 
	// @NOTE: The minus here is because in d3d texture coords start from top left corner, in gl bottom left corner
	projCoords.x = projCoords.x * 0.5 + 0.5f;
	projCoords.y = -projCoords.y * 0.5f + 0.5f;

	float currentDepth = projCoords.z;
	float bias = max(0.0005 * (1.0 - dot(worldNormal, lightDir)), 0.0005);
	//bias *= 1.0 / (cascades[layer] * 0.5f);
	//float bias = 0.001;

	currentDepth -= bias;

	float2 shadowMapSize;
	float numSlices;
	texture6.GetDimensions(shadowMapSize.x, shadowMapSize.y, numSlices);
	float2 texelSize = float2(1, 1) / shadowMapSize;

	float shadow = 0;
	shadow += currentDepth > texture6.Sample(pointSampler, float3(projCoords.xy + float2(-1, 1) * texelSize, layer)).r ? 1.0 : 0.0;
	shadow += currentDepth > texture6.Sample(pointSampler, float3(projCoords.xy + float2(0, 1) * texelSize, layer)).r ? 1.0 : 0.0;
	shadow += currentDepth > texture6.Sample(pointSampler, float3(projCoords.xy + float2(1, 1) * texelSize, layer)).r ? 1.0 : 0.0;

	shadow += currentDepth > texture6.Sample(pointSampler, float3(projCoords.xy + float2(-1, 0) * texelSize, layer)).r ? 1.0 : 0.0;
	shadow += currentDepth > texture6.Sample(pointSampler, float3(projCoords.xy + float2(0, 0) * texelSize, layer)).r ? 1.0 : 0.0;
	shadow += currentDepth > texture6.Sample(pointSampler, float3(projCoords.xy + float2(1, 0) * texelSize, layer)).r ? 1.0 : 0.0;

	shadow += currentDepth > texture6.Sample(pointSampler, float3(projCoords.xy + float2(-1, -1) * texelSize, layer)).r ? 1.0 : 0.0;
	shadow += currentDepth > texture6.Sample(pointSampler, float3(projCoords.xy + float2(0, -1) * texelSize, layer)).r ? 1.0 : 0.0;
	shadow += currentDepth > texture6.Sample(pointSampler, float3(projCoords.xy + float2(1, -1) * texelSize, layer)).r ? 1.0 : 0.0;
	shadow /= 9;

	//float shadow= 1.0 - texture6.SampleCmpLevelZero(shadowPCFSampler, float3(projCoords.xy, layer), currentDepth);

	return shadow;

#else
	float4 lightPos = mul(float4(worldPos, 1.0), lightVP[0]);

	// @NOTE: this divide by w is meaninless when useing ortho
	float3 projCoords = lightPos.xyz / lightPos.w;

	// @NOTE: No change in ZZZZZZZZ because in d3d depth is [0, 1] and in gl [-1, 1] 
	// @NOTE: The minus here is because in d3d texture coords start from top left corner, in gl bottom left corner
	projCoords.x = projCoords.x * 0.5 + 0.5f;
	projCoords.y = -projCoords.y * 0.5f + 0.5f;

	float closestDepth = texture6.Sample(pointSampler, float3(projCoords.xy, 0)).r;
	float currentDepth = projCoords.z;
	float bias = 0.001f;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	return shadow;
#endif
}
// ----------------------------------------------------------------------------
float3 CalculateDirectionalLighting(PixelInfo info, Material material, DirectionalLight light)
{
	// calculate per-light radiance
	float3 N = info.normal;
	float3 V = info.toView;

	float3 L = normalize(-light.direction);
	float3 H = normalize(V + L);

	float3 debugColour;
	float shadow = 1.0 - ShadowCalculation(info.worldPos, info.normal, L);
	//return float3(shadow, shadow, shadow);

	float3 radiance = shadow * light.colour * 1.0f;

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, material.roughness);
	float G = GeometrySmith(N, V, L, material.roughness);
	float3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), material.F0);

	// Combining
	float3 nominator = NDF * G * F;
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	float3 specular = nominator / max(denominator, 0.00001); // prevent divide by zero for NdotV=0.0 or NdotL=0.0

	// kS is equal to Fresnel
	float3 kS = F;

	// for energy conservation, the diffuse and specular light can't
	// be above 1.0 (unless the surface emits light); to preserve this
	// relationship the diffuse component (kD) should equal 1.0 - kS.
	float3 kD = float3(1.0, 1.0, 1.0) - kS;

	// multiply kD by the inverse metalness such that only non-metals 
	// have diffuse lighting, or a linear blend if partly metal (pure metals
	// have no diffuse light).
	kD *= 1.0 - material.metallic;

	// scale light by NdotL
	float NdotL = max(dot(N, L), 0.0);

	// note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	return (kD * material.albedo / PI + specular) * radiance * NdotL;
}
// ----------------------------------------------------------------------------
float3 CalculateSpotLighting(PixelInfo info, Material material, SpotLight light)
{
	// calculate per-light radiance
	float3 N = info.normal;
	float3 V = info.toView;

	float3 L = normalize(light.position - info.worldPos);
	float3 H = normalize(V + L);

	float theta = dot(L, normalize(-light.direction));
	float cutoff = light.inner;
	float outerCutOff = light.outter;
	float epsilon = cutoff - outerCutOff;
	float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

	float distance = length(light.position - info.worldPos);
	float attenuation = 1.0 / (distance * distance); // @NOTE: Use dot for sqrd 
	float3 radiance = light.colour * attenuation;

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, material.roughness);
	float G = GeometrySmith(N, V, L, material.roughness);
	float3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), material.F0);

	// Combining
	float3 nominator = NDF * G * F;
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	float3 specular = nominator / max(denominator, 0.00001); // prevent divide by zero for NdotV=0.0 or NdotL=0.0

	// kS is equal to Fresnel
	float3 kS = F;

	// for energy conservation, the diffuse and specular light can't
	// be above 1.0 (unless the surface emits light); to preserve this
	// relationship the diffuse component (kD) should equal 1.0 - kS.
	float3 kD = float3(1.0, 1.0, 1.0) - kS;

	// multiply kD by the inverse metalness such that only non-metals 
	// have diffuse lighting, or a linear blend if partly metal (pure metals
	// have no diffuse light).
	kD *= 1.0 - material.metallic;

	// scale light by NdotL
	float NdotL = max(dot(N, L), 0.0);

	// note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	return ((kD * material.albedo / PI + specular) * radiance * NdotL) * intensity;
}
// ----------------------------------------------------------------------------
float3 CalculatePointLighting(PixelInfo info, Material material, PointLight light)
{
	// calculate per-light radiance
	float3 N = info.normal;
	float3 V = info.toView;

	float3 L = normalize(light.position - info.worldPos);
	float3 H = normalize(V + L);

	float distance = length(light.position - info.worldPos);
	float attenuation = 1.0 / (distance * distance); // @NOTE: Use dot for sqrd 
	float3 radiance = light.colour * attenuation;

	// Cook-Torrance BRDF
	float NDF = DistributionGGX(N, H, material.roughness);
	float G = GeometrySmith(N, V, L, material.roughness);
	float3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), material.F0);

	// Combining
	float3 nominator = NDF * G * F;
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	float3 specular = nominator / max(denominator, 0.00001); // prevent divide by zero for NdotV=0.0 or NdotL=0.0

	// kS is equal to Fresnel
	float3 kS = F;

	// for energy conservation, the diffuse and specular light can't
	// be above 1.0 (unless the surface emits light); to preserve this
	// relationship the diffuse component (kD) should equal 1.0 - kS.
	float3 kD = float3(1.0, 1.0, 1.0) - kS;

	// multiply kD by the inverse metalness such that only non-metals 
	// have diffuse lighting, or a linear blend if partly metal (pure metals
	// have no diffuse light).
	kD *= 1.0 - material.metallic;

	// scale light by NdotL
	float NdotL = max(dot(N, L), 0.0);

	// note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	return (kD * material.albedo / PI + specular) * radiance * NdotL;
}
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
[earlydepthstencil]
float4 main(VSInput vsInput) : SV_TARGET
{
	PixelInfo pinfo;
	pinfo.normal = normalize(vsInput.normal);
	pinfo.worldPos = vsInput.world_position;
	pinfo.toView = viewPos - pinfo.worldPos;
	//pinfo.lightViewProj = vsInput.lightViewProj;
	//pinfo.lightView = vsInput.lightView;

	Material material;
	//material.albedo = float3(1, 0, 0);//pow(tex.Sample(splr, texture_coords).rgb, float3(2.2, 2.2, 2.2));
	material.albedo = pow(texture0.Sample(linearSampler, vsInput.texture_coords).rgb, float3(2.2, 2.2, 2.2));

#if 0
	float3 orm = texture1.Sample(linearSampler, vsInput.texture_coords).rgb;
	float ao = orm.r;
	material.roughness = orm.g;
	material.metallic = orm.b;
	pinfo.normal = HackGetNormalFromMap(texture2, vsInput.world_position, vsInput.normal, vsInput.texture_coords);
#else
	float ao = 1;
	material.roughness = 0.5f;
	material.metallic = 0.0f;
#endif

	material.F0 = float3(0.04, 0.04, 0.04); //(like plastic)

	float3 Lo = float3(0,0,0);

	DirectionalLight dlight;
	dlight.direction = directionalLights[0].direction;
	dlight.colour = directionalLights[0].colour;

	//dlight.direction = normalize(float3(1.0f, -1.0f, -1.0f));
	//dlight.colour = float3(1.0, 1.0, 1.0) * 10;

	Lo += CalculateDirectionalLighting(pinfo, material, dlight);

	//for (int dindex = 0; dindex < directionalLightCount; dindex++)
	//{
	//	DirectionalLight dlight;
	//	dlight.direction = directionalLights[dindex].direction;
	//	dlight.colour = directionalLights[dindex].colour;

	//	Lo += CalculateDirectionalLighting(pinfo, material, dlight);
	//}

	//for (int sindex = 0; sindex < spotLightCount; sindex++)
	//{
	//	SpotLight slight;
	//	slight.position = spotLights[sindex].position.xyz;
	//	slight.direction = spotLights[sindex].direction.xyz;

	//	//slight.inner = cos(0.20944f);
	//	//slight.outter = cos(0.261799f);

	//	slight.inner = spotLights[sindex].position.w;
	//	slight.outter = spotLights[sindex].direction.w;

	//	slight.colour = spotLights[sindex].colour;
	//	Lo += CalculateSpotLighting(pinfo, material, slight);
	//}

	//for (int pindex = 0; pindex < pointLightCount; pindex++)
	//{
	//	PointLight plight;
	//	plight.colour = pointLights[pindex].colour * 100;
	//	plight.position = pointLights[pindex].position;
	//	Lo += CalculatePointLighting(pinfo, material, plight);
	//}

	float3 kS = fresnelSchlickRoughness(max(dot(pinfo.normal, pinfo.toView), 0.0), material.F0, material.roughness);
	float3 kD = (1.0 - kS) * (1.0 - material.metallic);
	float3 diffuse = cubeTexture5.Sample(linearSampler, pinfo.normal).rgb * material.albedo;
	float3 ambient = kD * diffuse * ao;

	float3 color = Lo + ambient;

	// Gamma
	color = color / (color + float3(1.0, 1.0, 1.0));
	color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));

	if (USE_DEBUG)
	{
		float4 pixel_colour = DEBUG_COLOUR * float4(color, 1.0);
		return pixel_colour;
	}
	else
	{
		float4 pixel_colour = float4(color, 1.0);
		return pixel_colour;
	}


	//texture0.Sample(linearSampler, vsInput.texture_coords).rgb;
}