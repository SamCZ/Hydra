#pragma hydra vert:MainVS pixel:MainPS

#include "Assets/Shaders/Input/Default.hlsli"

struct PS_Input
{
	float4 position   : SV_Position;
	float3 positionLS : WSPOSITION1;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float3 color : COLOR;
};

cbuffer Globals
{
	float4x4 _ProjectionMatrix;
	float4x4 _ViewMatrix;
	float4x4 _ModelMatrix;
};

PS_Input MainVS(in VS_Input input, uint id : SV_VertexID)
{
	PS_Input OUT;

	OUT.position = mul(mul(mul(_ProjectionMatrix, _ViewMatrix), _ModelMatrix), float4(input.position.xyz, 1.0));

	OUT.positionLS = input.position.xyz;

	OUT.normal = normalize(input.normal);
	OUT.uv = input.texCoord;
	OUT.color = input.color;

	return OUT;
}

Texture2D _TerrainTexture : register(t0);

SamplerState _PixelSamplera
{
	MipFilter = LINEAR;
	MinFilter = NEAREST;
	MagFilter = NEAREST;
};

float4 MainPS(PS_Input IN) : SV_Target
{
	float d = dot(float3(0.5, 1, 0.5), IN.normal);
	d = clamp(d, 0.5, 1.0);
	//d = 1.0;

	//float3 color = _GrassTex.Sample(_PixelSamplera, IN.uv).xyz;
	float3 color = _TerrainTexture[IN.uv * 256].xyz;

	float lightLevel = IN.color.x / 256.0;
	lightLevel = clamp(lightLevel, 0.25, 1.0);
	
	d *= lightLevel;

	return float4(d * color.r, d * color.g, d * color.b, 1.0);
}