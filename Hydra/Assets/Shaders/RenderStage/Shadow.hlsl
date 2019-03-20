#include "Assets/Shaders/Input/Default.hlsli"

#pragma hydra vert:MainVS pixel:MainPS

struct PS_Input
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
};

cbuffer GlobalConstants : register(b0)
{
	float4x4 g_ProjectionMatrix;
	float4x4 g_ViewMatrix;
}

cbuffer ModelConstants : register(b1)
{
	float4x4 g_ModelMatrix;
}

static float4x4 Identity = {
	{ 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 0, 1 }
};

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

	output.position = mul(mul(mul(g_ProjectionMatrix, g_ViewMatrix), modelMatrix), output.position);

	output.normal = input.normal;

	return output;
}

float4 MainPS(PS_Input input) : SV_Target0
{
	return float4(input.normal, 1.0);
}