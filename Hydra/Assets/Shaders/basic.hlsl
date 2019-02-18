#pragma pack_matrix( column_major )

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
	float4 positionWS : WSPOSITION;
	float2 texCoord : TEXCOORD;
	float3 normal   : NORMAL;
	float3 color    : COLOR;
};

cbuffer GlobalConstants : register(b0)
{
	float4x4 g_ProjectionMatrix;
	float4x4 g_ViewMatrix;
	float4x4 g_ModelMatrix;
	float3x3 g_NormalMatrix;
	float2 space0;
	float3 g_TestColor;
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

	float4x4 modelMatrix = mul(instanceMatrix, g_ModelMatrix);

	float4 viewPos = mul(mul(g_ViewMatrix, modelMatrix), float4(input.position.xyz, 1.0f));

	output.position = mul(g_ProjectionMatrix, viewPos);
	output.positionWS = viewPos;
	output.texCoord = input.texCoord;

	output.normal = input.normal;

	output.color = g_TestColor;

	return output;
}

struct PS_Output
{
	float4 color : SV_Target0;
	float3 normal : SV_Target1;
	float4 position : SV_Target2;
};

PS_Output MainPS(PS_Input input) : SV_Target
{
	PS_Output output;

	float fl = (input.normal.x + input.normal.y + input.normal.z) / 3.0;
	fl = saturate(fl);

	//return Map_Albedo.Sample(DefaultSampler, input.texCoord);
	output.color = float4(input.color, 1.0);
	output.normal = input.normal;
	output.position = input.positionWS;

	return output;
}