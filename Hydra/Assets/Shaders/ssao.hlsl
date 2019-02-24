#pragma pack_matrix( column_major )

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

cbuffer Constants : register(b0)
{
	float4x4 g_Projection;
	float4 g_Samples[64];
}

cbuffer RuntimeConstants : register(b1)
{
	float4 g_Settings;
}

#define RADIUS g_Settings.x
#define BIAS g_Settings.y
#define PREVIEW g_Settings.z
#define INTENSITY g_Settings.w

Texture2D t_SceneTexture : register(t0);
Texture2D t_NormalTexture : register(t1);
Texture2D t_PositionTexture : register(t2);

static float2 noiseScale = float2(1280.0 / 4.0, 720.0 / 4.0);

static int kernelSize = 64;
/*static float radius = 0.085;
static float bias = 0.025;*/

float3 rand_2_10(in float2 uv)
{
	float noiseX = (frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
	float noiseY = sqrt(1 - noiseX * noiseX);

	return float3(noiseX, noiseY, 0);
}

float linearize_depth(float d, float zNear, float zFar)
{
	return zNear * zFar / (zFar + d * (zNear - zFar));
}

SamplerState MeshTextureSampler
{
	Filter = MIN_MAG_MIP_NEAREST;
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 MainPS(FullScreenQuadOutput IN) : SV_Target
{
	float3 fragPos = t_PositionTexture[IN.position.xy].xyz;
	float3 normal = (t_NormalTexture[IN.position.xy].xyz);
	//int index = (int)(IN.position.x * 1000.0) % 15;
	float3 randomVec = normalize((rand_2_10(IN.position.xy) * 2.0));
	
	float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	float3 bitangent = cross(normal, tangent);
	float3x3 TBN = float3x3(tangent, bitangent, normal);

	float occlusion = 0.0;

	for (int i = 0; i < kernelSize; ++i)
	{
		//float3 samplePos = mul(TBN, g_Samples[i].xyz);
		float3 samplePos = g_Samples[i].xyz;
		samplePos = fragPos + samplePos * RADIUS;
		//samplePos = fragPos;

		float4 offset = float4(samplePos, 1.0);
		offset = mul(g_Projection, offset);
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float sampleDepth = t_PositionTexture.Sample(MeshTextureSampler, float2(offset.x, 1.0 - offset.y)).z;
		//float sampleDepth = t_PositionTexture[float2(offset.x, 1.0 - offset.y)].z;

		float rangeCheck = smoothstep(0.0, 1.0, RADIUS / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + BIAS ? 1.0 : 0.0) * rangeCheck;

		occlusion *= INTENSITY;
	}
	occlusion = 1.0 - (occlusion / kernelSize);

	float4 color = t_SceneTexture[IN.position.xy];
	color.rgb *= occlusion;

	if(PREVIEW > 0.5)
		color.rgb = float3(occlusion, occlusion, occlusion);


	//color.rgb *= linearize_depth(t_PositionTexture[IN.position.xy].z, 0.1, 1000.0);

	//color.rgb = t_PositionTexture[IN.position.xy].xyz;

	//color.rgb = randomVec;

	return color;
}