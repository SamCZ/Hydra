#pragma hydra vert:MainVS pixel:MainPS

#include "Assets/Shaders/Input/Default.hlsli"

struct PS_Input
{
	float4 position   : SV_Position;
	float3 positionLS : WSPOSITION1;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
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

	return OUT;
}

Texture2D _GrassTex : register(t0);
SamplerState DefaultSampler	: register(s0);

float4 MainPS(PS_Input IN) : SV_Target
{
	float d = dot(float3(0.5, 1, 0.5), IN.normal);
	d = clamp(d, 0.5, 1.0);
	//d = 1.0;

	float3 color = _GrassTex.Sample(DefaultSampler, IN.uv).xyz;

	return float4(d * color.r, d * color.g, d * color.b, 1.0);
}