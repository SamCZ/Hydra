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

cbuffer GlobalConstants : register(b0)
{
	float3 ViewPos;
}

Texture2D AlbedoMetallic  : register(t0);
Texture2D NormalRoughness : register(t1);
Texture2D Additional      : register(t2);
Texture2D Depth           : register(t3);
Texture2D WorldPos        : register(t4);

TextureCube skyIR			: register(t5);
TextureCube skyPrefilter	: register(t6);
Texture2D brdfLUT			: register(t7);

SamplerState DefaultSampler	: register(s0);

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

float4 MainPS(FullScreenQuadOutput IN) : SV_Target
{
	GBUFFER_DATA data = DecodeGBuffer(IN);

	DirectionalLight DirLight;
	DirLight.Color = (1.0).xxxx;
	DirLight.Direction = normalize(float3(0.25, 0.8, 0.3));
	DirLight.Intensity = 1.0;

	AmbientLight Ambient;
	Ambient.Color = (1.0).xxxx;

	float3 albedo = data.Albedo;
	float metallic = data.Metallic;
	float roughness = data.Roughness;
	float ao = data.AO;

	float3 N = data.Normal;
	float3 V = normalize(ViewPos - data.WorldPos);
	float3 R = reflect(V, N);

	float3 F0 = (0.04).xxx;
	F0 = lerp(F0, albedo, metallic);

	// reflectance equation
	float3 Lo = (0.0).xxx;

	{
		// calculate per-light radiance
		float3 L = normalize(DirLight.Direction);
		float3 H = normalize(V + L);
		float3 radiance = DirLight.Color * DirLight.Intensity;

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
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
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

	//float globalIntensity = saturate(dot(float3(0.0, 1.0, 0.0), DirLight.Direction)).xxx;
	float globalIntensity = 1.0;
	float3 color = Lo + (ambient * globalIntensity);

	return float4(color, 1.0);
}