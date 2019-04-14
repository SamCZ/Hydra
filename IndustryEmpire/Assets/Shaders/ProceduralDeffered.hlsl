#pragma hydra vert:MainVS pixel:MainPS

#include "Assets/Shaders/Utils/Texturing.hlsli"

#pragma pack_matrix( column_major )

struct PS_Input
{
	float4 position     : SV_Position;
	float3 normal       : NORMAL;
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

	OUT.normal = normalize(_Buffer[id].Normal);

	return OUT;
}

struct PS_Attributes
{
	float4 AlbedoMetallic  : SV_Target0;
	float4 NormalRoughness : SV_Target1;
	float4 AOEmission      : SV_Target2;
	float4 WorldPos        : SV_Target3;
};

Texture2D _LayerTex0 : register(t0);
Texture2D _LayerTex1 : register(t1);
Texture2D _LayerTex2 : register(t2);
Texture2D _LayerTex3 : register(t3);
Texture2D _LayerTex4 : register(t4);
Texture2D _LayerTex5 : register(t5);
Texture2D _LayerTex6 : register(t6);

SamplerState DefaultSampler : register(s0);

PS_Attributes MainPS(PS_Input IN) : SV_Target
{
	PS_Attributes OUT;

	float3 color0 = TriplanarTexturing(_LayerTex1, DefaultSampler, IN.positionWS, IN.normal, 1.0);
	float3 color1 = TriplanarTexturing(_LayerTex0, DefaultSampler, IN.positionWS, IN.normal, 1.0);

	float3 color = lerp(color0, color1, pow(dot(IN.normal, float3(0, 1, 0)), 2) );

	OUT.AlbedoMetallic = float4(color, 0);
	OUT.NormalRoughness = float4(IN.normal, 1);

	OUT.WorldPos = float4(IN.positionWS, 1.0);

	return OUT;
}