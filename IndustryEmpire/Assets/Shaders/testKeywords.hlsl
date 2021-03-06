#pragma hydra vert:MainVS pixel:MainPS

struct FullScreenQuadOutput
{
	float4 position     : SV_Position;
	float2 uv           : TEXCOORD;
};

cbuffer Test01 : register(b0)
{
	float3 V0;
	float V1;
	float4x4 V3;
}

FullScreenQuadOutput MainVS(uint id : SV_VertexID)
{
	FullScreenQuadOutput OUT;

	uint u = ~id & 1;
	uint v = (id >> 1) & 1;
	OUT.uv = float2(u, v);
	OUT.position = float4(OUT.uv * 2 - 1, 0, 1);

	// In D3D (0, 0) stands for upper left corner
	OUT.uv.y = 1.0 - OUT.uv.y * V1;

	return OUT;
}

Texture2D t_SourceTexture : register(t0);

float4 MainPS(FullScreenQuadOutput IN) : SV_Target
{
	return t_SourceTexture[IN.position.xy];
}