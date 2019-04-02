#pragma hydra vert:MainVS pixel:MainPS

#pragma pack_matrix( column_major )

struct PS_Input
{
	float4 position     : SV_Position;
	float2 uv           : TEXCOORD;
	float3 positionWS : WSPOSITION0;
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

struct VoxelBuffer
{
	float4 Position;
	float3 Normal;
};

StructuredBuffer<VoxelBuffer> _Buffer : register(t0);

PS_Input MainVS(uint id : SV_VertexID)
{
	PS_Input OUT;

	float4 pos = _Buffer[id].Position;

	OUT.positionWS = mul(g_ModelMatrix, pos).xyz;

	OUT.position = mul(mul(mul(g_ProjectionMatrix, g_ViewMatrix), g_ModelMatrix), pos);

	return OUT;
}

struct PS_Attributes
{
	float4 AlbedoMetallic  : SV_Target0;
	float4 NormalRoughness : SV_Target1;
	float4 AOEmission      : SV_Target2;
	float4 WorldPos        : SV_Target3;
};

PS_Attributes MainPS(PS_Input IN) : SV_Target
{
	PS_Attributes OUT;

	OUT.AlbedoMetallic = float4(1, 1, 1, 0);
	OUT.NormalRoughness = float4(0, 1, 0, 1);

	OUT.WorldPos = float4(IN.positionWS, 1.0);

	return OUT;
}