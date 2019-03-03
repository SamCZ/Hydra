#include "Assets/Shaders/Input/Default.hlsli"

struct PS_Input
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 positionWS : WSPOSITION;
};

cbuffer GlobalConstants : register(b0)
{
	float4x4 g_ProjectionMatrix;
	float4x4 g_ViewMatrix;
}

cbuffer ModelConstants : register(b1)
{
	float4x4 g_ModelMatrix;
	float g_Opacity;
}

static float4x4 Identity = {
	{ 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 0, 1 }
};

PS_Input OnMainVS(in VS_Input input, in PS_Input output);

PS_Input MainVS(VS_Input input, unsigned int InstanceID : SV_InstanceID)
{
	PS_Input output;

	float4x4 instanceMatrix = input.instanceMatrix;
	if (instanceMatrix[0][0] == 0 && instanceMatrix[0][1] == 0 && instanceMatrix[0][2] == 0 && instanceMatrix[0][3] == 0 &&
		instanceMatrix[1][0] == 0 && instanceMatrix[1][1] == 0 && instanceMatrix[1][2] == 0 && instanceMatrix[1][3] == 0 &&
		instanceMatrix[2][0] == 0 && instanceMatrix[2][1] == 0 && instanceMatrix[2][2] == 0 && instanceMatrix[2][3] == 0 &&
		instanceMatrix[3][0] == 0 && instanceMatrix[3][1] == 0 && instanceMatrix[3][2] == 0 && instanceMatrix[3][3] == 0)
	{
		instanceMatrix = Identity;
	}
	float4x4 modelMatrix = mul(instanceMatrix, g_ModelMatrix);
	output.position = float4(input.position.xyz, 1.0f);

	output = OnMainVS(input, output);

	output.position = mul(mul(mul(g_ProjectionMatrix, g_ViewMatrix), modelMatrix), output.position);
	output.positionWS = mul(modelMatrix, float4(input.position.xyz, 1.0)).xyz;

	output.texCoord = input.texCoord;
	output.normal = input.normal;
	output.tangent = mul(modelMatrix, float4(input.tangent, 0.0)).xyz;
	output.binormal = mul(modelMatrix, float4(input.binormal, 0.0)).xyz;

	return output;
}

struct PS_Attributes
{
	float4 AlbedoMetallic  : SV_Target0;
	float4 NormalRoughness : SV_Target1;
	float4 AOEmission      : SV_Target2;
	float4 WorldPos        : SV_Target3;
};

struct PBROutput
{
	float3 Albedo;
	float3 Normal;
	float Metallic;
	float Roughness;
	float AO;
	float3 Emission;
};

PBROutput OnMainPS(in PS_Input input);

PS_Attributes MainPS(PS_Input input) : SV_Target {
	PS_Attributes output;

	output.WorldPos = float4(input.positionWS, 1.0);

	PBROutput pbrData = OnMainPS(input);

	output.AlbedoMetallic = float4(pbrData.Albedo, pbrData.Metallic);
	output.NormalRoughness = float4(pbrData.Normal, pbrData.Roughness);
	output.AOEmission = float4(pbrData.AO, pbrData.Emission.r, pbrData.Emission.g, pbrData.Emission.b);

	return output;
}