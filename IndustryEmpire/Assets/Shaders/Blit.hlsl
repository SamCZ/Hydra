#pragma hydra vert:MainVS pixel:MainPS

struct FullScreenQuadOutput
{
	float4 position     : SV_Position;
	float2 uv           : TEXCOORD;
};

FullScreenQuadOutput MainVS(uint id : SV_VertexID)
{
	FullScreenQuadOutput OUT;

	uint u = ~id & 1;
	uint v = (id >> 1) & 1;
	OUT.uv = float2(u, v);
	OUT.position = float4(OUT.uv * 2 - 1, 0, 1);

	// In D3D (0, 0) stands for upper left corner
	OUT.uv.y = 1.0 - OUT.uv.y;

	return OUT;
}

Texture2D _Texture : register(t0);

float4 MainPS(FullScreenQuadOutput IN) : SV_Target
{
	float4 color = _Texture[IN.position.xy];

	clip(color.a - 0.01);

	return color;
}