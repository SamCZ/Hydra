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