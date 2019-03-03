static const float PI = 3.14159265359;

SamplerState SamplerAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 16;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct DirectionalLight
{
	float4 Color;
	//------------------------ 16 bytes
	float3 Direction;
	float Intensity;
	//------------------------

};

struct AmbientLight
{
	float4 Color;
	//-------------------------
};

struct PointLight
{
	float4 diffuseColor;
	//------------------------
	float3 position;
	float range;
	//-----------------------
	float3 attenuate;
	float pad;
	//-----------------------
};

struct SpotLight
{
	float4 diffuseColor;
	//-------------------------
	float3 position;
	float range;
	//-------------------------
	float3 direction;
	float spot;
	//-------------------------
	float3 attenuate;
	float pad;
	//-------------------------
};

cbuffer ExternalData : register(b1)
{
	/*DirectionalLight dirLight_1;
	AmbientLight ambientLight;
	SpotLight spotLight;
	float3 cameraPosition;
	float pointLightCount;
	
	PointLight pointLights[64];*/
};

float NormalDistributionGGXTR(float3 normalVec, float3 halfwayVec, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;   // a2 = a^2
	float NdotH = max(dot(normalVec, halfwayVec), 0.0);     // NdotH = normalVec.halfwayVec
	float NdotH2 = NdotH * NdotH;   // NdotH2 = NdotH^2
	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
	denom = PI * denom * denom;

	return nom / denom;
}


float GeometrySchlickGGX(float NdotV, float roughness)  // k is a remapping of roughness based on direct lighting or IBL lighting
{
	float r = roughness + 1.0f;
	float k = (r * r) / 8.0f;

	float nom = NdotV;
	float denom = NdotV * (1.0f - k) + k;

	return nom / denom;
}


float GeometrySmith(float3 normalVec, float3 viewDir, float3 lightDir, float k)
{
	float NdotV = max(dot(normalVec, viewDir), 0.0f);
	float NdotL = max(dot(normalVec, lightDir), 0.0f);
	float ggx1 = GeometrySchlickGGX(NdotV, k);
	float ggx2 = GeometrySchlickGGX(NdotL, k);

	return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)   // cosTheta is n.v and F0 is the base reflectivity
{
	return (F0 + (1.0f - F0) * pow(1.0 - cosTheta, 5.0f));
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)   // cosTheta is n.v and F0 is the base reflectivity
{
	return F0 + (max(float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0f);
}