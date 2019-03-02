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

Texture2D AlbedoMetallic  : register(t0);
Texture2D NormalRoughness : register(t1);
Texture2D Additional      : register(t2);
Texture2D Depth           : register(t3);
Texture2D WorldPos        : register(t4);

struct GBUFFER_DATA
{
	float3 Normal;
	float3 WorldPos;
	float3 Albedo;
	float Metallic;
	float Roughness;
	float AO;
};

GBUFFER_DATA DecodeGBuffer(FullScreenQuadOutput IN)
{
	GBUFFER_DATA data;
	float4 pack1 = AlbedoMetallic[IN.position.xy];
	float4 pack2 = NormalRoughness[IN.position.xy];
	float4 pack3 = Additional[IN.position.xy];

	data.Normal = normalize(pack2.xyz);
	data.WorldPos = WorldPos[IN.position.xy].xyz;
	data.Albedo = pack1.rgb;
	data.Metallic = pack1.a;
	data.Roughness = pack2.a;
	data.AO = pack3.r;

	return data;
}

float4 MainPS(FullScreenQuadOutput IN) : SV_Target
{
	GBUFFER_DATA data = DecodeGBuffer(IN);

	float3 albedo = data.Albedo;
	float metallic = data.Metallic;
	float roughness = data.Roughness;
	float ao = data.AO;

	return float4(albedo, 1.0);
}