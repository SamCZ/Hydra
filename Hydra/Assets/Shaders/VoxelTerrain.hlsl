#include "Assets/Shaders/RenderStage/Deffered.hlsli"
#include "Assets/Shaders/Utils/Texturing.hlsli"

#pragma hydra vert:MainVS pixel:MainPS
#pragma hydra rs:Deffered

//Define keywords
#pragma hydra kw:pixel:USE_ALBEDO_TEX
#pragma hydra kw:pixel:USE_NORMAL_TEX
#pragma hydra kw:pixel:USE_ROUGHNESS_TEX
#pragma hydra kw:pixel:USE_METALLIC_TEX
#pragma hydra kw:pixel:USE_AO_TEX
#pragma hydra kw:pixel:USE_EMISSION_TEX

Texture2D _GrassTex : register(t0);
Texture2D _GrassNormalTex : register(t1);

Texture2D _LayerTex0 : register(t2);
Texture2D _LayerTex1 : register(t3);
Texture2D _LayerTex2 : register(t4);
Texture2D _LayerTex3 : register(t5);
Texture2D _LayerTex4 : register(t6);
Texture2D _LayerTex5 : register(t7);
Texture2D _LayerTex6 : register(t8);

SamplerState DefaultSampler	: register(s0);

Texture2D _HeightMap : register(t0);

cbuffer ModelData : register(b0)
{
	float3 _Color;
	float _TexScale;
}

cbuffer VertexBufferVars
{
	int g_numVertsPerLine;
	int g_skipIncrement;
}

PS_Input OnMainVS(in VS_Input input, in PS_Input output)
{
	//float4 clipPlane = float4(0, 10, 0, -1);
	//output.clip = dot(mul(g_ModelMatrix, input.position), clipPlane);
	uint2 pos_xy = (uint2)input.texCoord2;

	float x = pos_xy.x;
	float y = pos_xy.y;

	float height = _HeightMap[pos_xy];

	int numVertsPerLine = g_numVertsPerLine;
	int skipIncrement = g_skipIncrement;

	bool isOutOfMeshVertex = y == 0 || y == numVertsPerLine - 1 || x == 0 || x == numVertsPerLine - 1;
	bool isMeshEdgeVertex = (y == 1 || y == numVertsPerLine - 2 || x == 1 || x == numVertsPerLine - 2) && !isOutOfMeshVertex;
	bool isMainVertex = (x - 2) % skipIncrement == 0 && (y - 2) % skipIncrement == 0 && !isOutOfMeshVertex && !isMeshEdgeVertex;
	bool isEdgeConnectionVertex = (y == 2 || y == numVertsPerLine - 3 || x == 2 || x == numVertsPerLine - 3) && !isOutOfMeshVertex && !isMeshEdgeVertex && !isMainVertex;

	if (isEdgeConnectionVertex)
	{
		bool isVertical = x == 2 || x == numVertsPerLine - 3;
		int dstToMainVertexA = ((isVertical) ? y - 2 : x - 2) % skipIncrement;
		int dstToMainVertexB = skipIncrement - dstToMainVertexA;
		float dstPercentFromAToB = dstToMainVertexA / (float)skipIncrement;

		float heightMainVertexA = _HeightMap[uint2((isVertical) ? x : x - dstToMainVertexA, (isVertical) ? y - dstToMainVertexA : y)];
		float heightMainVertexB = _HeightMap[uint2((isVertical) ? x : x + dstToMainVertexB, (isVertical) ? y + dstToMainVertexB : y)];

		height = heightMainVertexA * (1 - dstPercentFromAToB) + heightMainVertexB * dstPercentFromAToB;
	}

	output.position.y = height;

	return output;
}

PBROutput OnMainPS(in PS_Input input)
{
	PBROutput output;

	float texScale = _TexScale;

	float3 grassColor = TriplanarTexturing(_LayerTex0, DefaultSampler, input.positionWS, input.normal, texScale).rgb;
	float3 rocks1Color = TriplanarTexturing(_LayerTex1, DefaultSampler, input.positionWS, input.normal, texScale).rgb;
	float3 rocks2Color = TriplanarTexturing(_LayerTex2, DefaultSampler, input.positionWS, input.normal, texScale).rgb;
	float3 sandyGrassColor = TriplanarTexturing(_LayerTex3, DefaultSampler, input.positionWS, input.normal, texScale).rgb;
	float3 snowColor = TriplanarTexturing(_LayerTex4, DefaultSampler, input.positionWS, input.normal, texScale).rgb;
	float3 stonyGroudColor = TriplanarTexturing(_LayerTex5, DefaultSampler, input.positionWS, input.normal, texScale).rgb;
	float3 waterColor = TriplanarTexturing(_LayerTex6, DefaultSampler, input.positionWS, input.normal, texScale).rgb;

	float topDot = dot(input.normal, float3(0, 1, 0));

	float rightOffset = 0.1;

	float v = smoothstep(rightOffset, 0.5, topDot) * smoothstep(1.0 - rightOffset, 0.5, topDot);
	
	float3 ridges = lerp(rocks1Color, rocks2Color, v);

	output.Albedo = grassColor;
	//output.Albedo = _Color;
	output.Normal = TriplanarTexturingNormal(input, _GrassNormalTex, DefaultSampler, 1.0);//GetNormalFromNormalMap(input, _GrassNormalTex, DefaultSampler, input.positionWS.xz);
	output.Metallic = 0.0;
	output.Roughness = 1.0;
	output.AO = 0.0;

	return output;
}