#pragma hydra vert:MainVS pixel:MainPS

#pragma pack_matrix( column_major )

struct FullScreenQuadOutput
{
	float4 position     : SV_Position;
	float2 uv           : TEXCOORD;
};

cbuffer Globals
{
	float4x4 _ProjectionMatrix;
	float4x4 _ViewMatrix;
	float4x4 _ModelMatrix;
};

FullScreenQuadOutput MainVS(uint id : SV_VertexID)
{
	FullScreenQuadOutput OUT;

	uint u = ~id & 1;
	uint v = (id >> 1) & 1;
	OUT.uv = float2(u, v);
	OUT.position = mul(mul(_ProjectionMatrix, _ViewMatrix), mul(_ModelMatrix, float4((OUT.uv * 2 - 1) * 50.0, 0, 1)));

	// In D3D (0, 0) stands for upper left corner
	OUT.uv.y = 1.0 - OUT.uv.y;

	return OUT;
}

Texture2D _Texture : register(t0);

cbuffer Yo
{
	float3 _Color;
};

float4 MainPS(FullScreenQuadOutput IN) : SV_Target
{
	float4 color = float4(_Color, 1);

	color.x = frac(IN.uv.x * 10);
	color.y = frac(IN.uv.y * 10);

	return color;
}