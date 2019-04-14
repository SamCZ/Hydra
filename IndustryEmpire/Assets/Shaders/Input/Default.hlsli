#pragma pack_matrix( column_major )

struct VS_Input
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD0;
	float2 texCoord2 : TEXCOORD1;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float4x4 instanceMatrix : WORLD;
};