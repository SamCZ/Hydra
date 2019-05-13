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
	//OUT.uv.y = 1.0 - OUT.uv.y;

	return OUT;
}

static float weight[] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

static float BlurWeights[13] =
{
	1.0f / 4096.0f,
	12.0f / 4096.0f,
	66.0f / 4096.0f,
	220.0f / 4096.0f,
	495.0f / 4096.0f,
	792.0f / 4096.0f,
	924.0f / 4096.0f,
	792.0f / 4096.0f,
	495.0f / 4096.0f,
	220.0f / 4096.0f,
	66.0f / 4096.0f,
	12.0f / 4096.0f,
	1.0f / 4096.0f,
};

cbuffer BasicBuffer : register(b0)
{
	float2 _Direction;
	float2 _TexSize;
};

SamplerState PointSampler
{
	Filter = MIN_MAG_MIP_NEAREST;
	AddressU = Clamp;
	AddressV = Clamp;
};

Texture2D _Texture : register(t0);

static const float PI = 3.14159265359;



float4 MainPS(FullScreenQuadOutput IN) : SV_Target
{
	float2 tex_offset = 1.0 / _TexSize;

	/*float4 color = _Texture.Sample(PointSampler, IN.uv);

	float intensity = (color.r + color.g + color.b) / 3.0;

	float blurSize = 20.0;
	float sigma = 2.0;

	float numBlurPixelsPerSide = blurSize / 2.0;

	float2 blurMultiplyVec = _Direction;

	float3 incrementalGaussian = float3(0.0, 0.0, 0.0);
	incrementalGaussian.x = 1.0 / (sqrt(2.0 * PI) * sigma);
	incrementalGaussian.y = exp(-0.5 / (sigma * sigma));
	incrementalGaussian.z = incrementalGaussian.y * incrementalGaussian.y;

	float4 avgValue = float4(0.0, 0.0, 0.0, 0.0);
	float coefficientSum = 0.0;

	avgValue += color * incrementalGaussian.x;
	coefficientSum += incrementalGaussian.x;
	incrementalGaussian.xy *= incrementalGaussian.yz;

	for (float i = 1.0; i <= numBlurPixelsPerSide; i++)
	{
		avgValue += _Texture.Sample(PointSampler, IN.uv - i * tex_offset * blurMultiplyVec) * incrementalGaussian.x;
		avgValue += _Texture.Sample(PointSampler, IN.uv + i * tex_offset * blurMultiplyVec) * incrementalGaussian.x;

		coefficientSum += 2.0 * incrementalGaussian.x;
		incrementalGaussian.xy *= incrementalGaussian.yz;
	}

	return avgValue / coefficientSum;*/

	float3 color = _Texture.Sample(PointSampler, IN.uv).rgb;

	float intensity = (color.r + color.g + color.b) / 3.0;

	float3 result = color * weight[0];
	
	if (_Direction.x > 0.5)
	{
		for (int i = 1; i < 5; ++i)
		{
			result += _Texture.Sample(PointSampler, IN.uv + float2(tex_offset.x * i, 0.0)).rgb * weight[i];
			result += _Texture.Sample(PointSampler, IN.uv - float2(tex_offset.x * i, 0.0)).rgb * weight[i];
		}
	}

	if (_Direction.y > 0.5)
	{
		for (int i = 1; i < 5; ++i)
		{
			result += _Texture.Sample(PointSampler, IN.uv + float2(0.0, tex_offset.y * i)).rgb * weight[i];
			result += _Texture.Sample(PointSampler, IN.uv - float2(0.0, tex_offset.y * i)).rgb * weight[i];
		}
	}

	return float4(result, 1.0);
}