#pragma hydra vert:MainVS pixel:MainPS

#include "Assets/Shaders/Input/Default.hlsli"

struct PS_Input
{
	float4 position		: SV_POSITION;
	float3 positionWS   : WSPOSITION;
	float3 uvw			: TEXCOORD;
};

cbuffer externalData : register(b0)
{
	float4x4 g_ProjectionMatrix;
	float4x4 g_ViewMatrix;
};

PS_Input MainVS(VS_Input input)
{
	PS_Input output;


	output.position = mul(g_ProjectionMatrix, mul(g_ViewMatrix, float4(input.position, 1.0f)));
	output.position.z = output.position.w;
	output.uvw = input.position;
	output.positionWS = input.position;

	return output;
}

TextureCube _Texture : register(t0);
SamplerState _DefaultSampler	: register(s0);

static float PI = 3.14159265359;

float4 MainPS(PS_Input input) : SV_TARGET
{
	float3 N = normalize(input.positionWS);

	float3 irradiance = (0.0).xxx;

	// tangent space calculation from origin point
	float3 up = float3(0.0, 1.0, 0.0);
	float3 right = cross(up, N);
	up = cross(N, right);

	float sampleDelta = 0.025;
	float nrSamples = 0.0f;
	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// spherical to cartesian (in tangent space)
			float3 tangentSample = float3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

			irradiance += _Texture.Sample(_DefaultSampler, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(nrSamples));

	return float4(irradiance, 1.0);
}