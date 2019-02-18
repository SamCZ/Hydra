//#pragma pack_matrix( column_major )

struct VS_Input
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float4x4 instanceMatrix : WORLD;
};

struct PS_Input
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};

cbuffer GlobalConstants : register(b0)
{
	float4x4 g_ProjectionMatrix;
	float4x4 g_ViewMatrix;
	//float4x4 g_ModelMatrix;
}

static float4x4 Identity = {
	{ 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 0, 1 }
};

SamplerState DefaultSampler : register(s0);
Texture2D Map_Albedo : register(t0);

PS_Input MainVS(VS_Input input, unsigned int InstanceID : SV_InstanceID, uint id : SV_VertexID)
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

	float4x4 modelMatrix = Identity;

	output.position = mul(mul(mul(g_ProjectionMatrix, g_ViewMatrix), modelMatrix), float4(input.position.xyz, 1.0f));
	//output.position = mul(g_ProjectionMatrix, float4(input.position.xyz, 1.0f));
	output.texCoord = input.texCoord;

	return output;
}

float4 MainPS(PS_Input input) : SV_Target0
{
	//return Map_Albedo.Sample(DefaultSampler, input.texCoord);
	return float4(1.0, 1.0, 1.0, 1.0);
}