#pragma hydra vert:MainVS pixel:MainPS

#include "Assets/Shaders/Utils/PBRHeader.hlsli"

struct FullScreenQuadOutput
{
	float4 position     : SV_Position;
	float2 uv           : TEXCOORD;
};

FullScreenQuadOutput MainVS(uint id : SV_VertexID)
{
	FullScreenQuadOutput OUT;

	uint u = ~id & 1;
	uint v = (id >> 1) & 1;
	OUT.uv = float2(u, v);
	OUT.position = float4(OUT.uv * 2 - 1, 0, 1);

	// In D3D (0, 0) stands for upper left corner
	OUT.uv.y = 1.0 - OUT.uv.y;

	return OUT;
}

cbuffer GlobalConstants : register(b1)
{
	float3 ViewPos;
}

cbuffer LightConstants : register(b2)
{
	float4x4 g_LightViewProjMatrix;
	float g_rShadowMapSize;
	float g_Bias;
	float2 _Pad001;
}

Texture2D AlbedoMetallic  : register(t0);
Texture2D NormalRoughness : register(t1);
Texture2D Additional      : register(t2);
Texture2D Depth           : register(t3);
Texture2D WorldPos        : register(t4);

TextureCube skyIR			: register(t5);
TextureCube skyPrefilter	: register(t6);
Texture2D brdfLUT			: register(t7);

Texture2D DirLight_ShadowMap : register(t8);

SamplerState DefaultSampler	: register(s0);
SamplerComparisonState ShadowSampler : register(s1);

struct GBUFFER_DATA
{
	float3 Normal;
	float3 WorldPos;
	float3 Albedo;
	float Metallic;
	float Roughness;
	float AO;
};

GBUFFER_DATA DecodeGBuffer(FullScreenQuadOutput IN)
{
	GBUFFER_DATA data;
	float4 pack1 = AlbedoMetallic[IN.position.xy];
	float4 pack2 = NormalRoughness[IN.position.xy];
	float4 pack3 = Additional[IN.position.xy];

	data.Normal = normalize(pack2.xyz);
	data.WorldPos = WorldPos[IN.position.xy].xyz;
	data.Albedo = pack1.rgb;
	data.Metallic = pack1.a;
	data.Roughness = pack2.a;
	data.AO = pack3.r;

	return data;
}

static const float2 g_SamplePositions[] = {
	// Poisson disk with 16 points
	float2(-0.3935238f, 0.7530643f),
	float2(-0.3022015f, 0.297664f),
	float2(0.09813362f, 0.192451f),
	float2(-0.7593753f, 0.518795f),
	float2(0.2293134f, 0.7607011f),
	float2(0.6505286f, 0.6297367f),
	float2(0.5322764f, 0.2350069f),
	float2(0.8581018f, -0.01624052f),
	float2(-0.6928226f, 0.07119545f),
	float2(-0.3114384f, -0.3017288f),
	float2(0.2837671f, -0.179743f),
	float2(-0.3093514f, -0.749256f),
	float2(-0.7386893f, -0.5215692f),
	float2(0.3988827f, -0.617012f),
	float2(0.8114883f, -0.458026f),
	float2(0.08265103f, -0.8939569f)
};

float GetShadow(float3 fragmentPos)
{
	float4 clipPos = mul(g_LightViewProjMatrix, float4(fragmentPos, 1.0f));

	if (abs(clipPos.x) > clipPos.w || abs(clipPos.y) > clipPos.w || clipPos.z <= 0)
	{
		return 0;
	}

	clipPos.xyz /= clipPos.w;
	clipPos.x = clipPos.x * 0.5 + 0.5;
	clipPos.y = 0.5 - clipPos.y * 0.5;

	if (g_DirLight.ShadowType < 2.0)
	{
		return DirLight_ShadowMap.SampleCmpLevelZero(ShadowSampler, clipPos.xy, clipPos.z);
	}

	float shadow = 0;
	float totalWeight = 0;

	for (int nSample = 0; nSample < 16; ++nSample)
	{
		float2 offset = g_SamplePositions[nSample];
		float weight = 1.0;
		offset *= 2 * g_rShadowMapSize;
		float smpl = DirLight_ShadowMap.SampleCmpLevelZero(ShadowSampler, clipPos.xy + offset, clipPos.z);

		shadow += smpl * weight;
		totalWeight += weight;
	}

	shadow /= totalWeight;
	shadow = pow(shadow, 2.2);

	return shadow;
}

float4 MainPS(FullScreenQuadOutput IN) : SV_Target
{
	GBUFFER_DATA data = DecodeGBuffer(IN);

	AmbientLight Ambient;
	Ambient.Color = (0.25).xxxx;

	if (data.WorldPos.z == 0)
	{
		return float4(0.0, 0.0, 0.0, 0.0);
	}

	float3 albedo = data.Albedo;
	float metallic = data.Metallic;
	float roughness = data.Roughness;
	float ao = data.AO;
	
	float shadowValue = 1.0;
	if (g_DirLight.ShadowType > 0.0)
	{
		shadowValue = GetShadow(data.WorldPos.xyz);
		shadowValue = max(shadowValue, 0.2);
	}

	float3 N = data.Normal;
	float3 V = normalize(ViewPos - data.WorldPos);
	float3 R = reflect(V, N);

	float3 F0 = (0.04).xxx;
	F0 = lerp(F0, albedo, metallic);

	// reflectance equation
	float3 Lo = (0.0).xxx;

	for (int i = 0; i < (int)g_PointLightCount; ++i)
	{
		// calculate per-light radiance
		float3 L = normalize(g_PointLights[i].Position - data.WorldPos);
		float3 H = normalize(V + L);
		float distance = length(g_PointLights[i].Position - data.WorldPos);
		float attenuation = 1.0 / (distance * distance);
		float3 radiance = g_PointLights[i].DiffuseColor * attenuation * g_PointLights[i].Range;

		// cook-torrance brdf
		float NDF = NormalDistributionGGXTR(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

		float3 kS = F;
		float3 kD = (1.0).xxx - kS;
		kD *= 1.0 - metallic;

		float3 nominator = NDF * G * F;
		float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
		float3 specular = nominator / denominator;

		// add to outgoing radiance Lo
		float NdotL = max(dot(N, L), 0.0);
		Lo += ((kD * albedo / PI + specular) * radiance * NdotL);
	}

	{
		// calculate per-light radiance
		float3 L = normalize(-g_DirLight.Direction);
		float3 H = normalize(V + L);
		float3 radiance = g_DirLight.Color.rgb * g_DirLight.Intensity;

		// cook-torrance brdf
		float NDF = NormalDistributionGGXTR(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

		float3 kS = F;
		float3 kD = (1.0).xxx - kS;
		kD *= 1.0 - metallic;

		float3 nominator = NDF * G * F;
		float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
		float3 specular = nominator / denominator;

		// add to outgoing radiance Lo
		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL * shadowValue;
	}

	// ambient lighting (we now use IBL as the ambient term)
	float3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

	float3 kS = F;
	float3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	float3 irradiance = skyIR.Sample(DefaultSampler, N * float3(1.0, -1.0, 1.0)).rgb;
	float3 diffuse = irradiance * albedo;

	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
	float3 prefilteredColor = skyPrefilter.SampleLevel(DefaultSampler, R, roughness * 4.0).rgb;
	float2 brdf = brdfLUT.Sample(SamplerAnisotropic, (max(dot(N, V), 0.0), roughness).xx).rg;
	float3 specular = prefilteredColor * (F * brdf.x + brdf.y);

	float3 ambient = (kD * diffuse + specular) * (Ambient.Color.xyz + ao.xxx);

	float globalIntensity = saturate(dot(float3(0.0, 1.0, 0.0), -g_DirLight.Direction)).xxx;
	//float globalIntensity = 1.0;
	float3 color = Lo + (ambient * globalIntensity);

	return float4(color, 1.0);
}