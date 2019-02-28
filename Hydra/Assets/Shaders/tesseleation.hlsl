#include "Assets/Shaders/Input/Deffered.hlsli"

#pragma hydra vert:ColorVertexShader hull:ColorHullShader dom:ColorDomainShader pixel:ColorPixelShader

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////

struct HullInputType
{
	float3 position : POSITION;
	float4 color : COLOR;
};


HullInputType ColorVertexShader(VS_Input input)
{
	HullInputType output;
	// Pass the vertex position into the hull shader.
	output.position = input.position;

	// Pass the input color into the hull shader.
	output.color = float4(input.normal.xyz, 1.0);
;

	return output;
}

////////////////////////////////////////////////////////////////////////////////
// Hull Shader
////////////////////////////////////////////////////////////////////////////////

cbuffer TessellationBuffer : register(b0)
{
	float tessellationAmount;
	float3 padding;
};

struct ConstantOutputType
{
	float edges[3] : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

struct HullOutputType
{
	float3 position : POSITION;
	float4 color : COLOR;
};

ConstantOutputType ColorPatchConstantFunction(InputPatch<HullInputType, 3> inputPatch, uint patchId : SV_PrimitiveID)
{
	ConstantOutputType output;

	// Set the tessellation factors for the three edges of the triangle.
	output.edges[0] = tessellationAmount;
	output.edges[1] = tessellationAmount;
	output.edges[2] = tessellationAmount;

	// Set the tessellation factor for tessallating inside the triangle.
	output.inside = tessellationAmount;

	return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ColorPatchConstantFunction")]
HullOutputType ColorHullShader(InputPatch<HullInputType, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	HullOutputType output;


	// Set the position for this control point as the output position.
	output.position = patch[pointId].position;

	// Set the input color as the output color.
	output.color = patch[pointId].color;

	return output;
}

////////////////////////////////////////////////////////////////////////////////
// Domain Shader
////////////////////////////////////////////////////////////////////////////////

cbuffer GlobalConstants : register(b0)
{
	float4x4 g_ProjectionMatrix;
	float4x4 g_ViewMatrix;
	float4x4 g_ModelMatrix;
	float3x3 g_NormalMatrix;
	float2 space0;
	float3 g_TestColor;
}

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

[domain("tri")]
PixelInputType ColorDomainShader(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<HullOutputType, 3> patch)
{
	float3 vertexPosition;
	PixelInputType output;


	// Determine the position of the new vertex.
	vertexPosition = uvwCoord.x * patch[0].position + uvwCoord.y * patch[1].position + uvwCoord.z * patch[2].position;

	float4 viewPos = mul(mul(g_ViewMatrix, g_ModelMatrix), float4(vertexPosition, 1.0f));

	output.position = mul(g_ProjectionMatrix, viewPos);

	// Calculate the position of the new vertex against the world, view, and projection matrices.
	/*output.position = mul(float4(vertexPosition, 1.0f), g_ModelMatrix);
	output.position = mul(output.position, g_ViewMatrix);
	output.position = mul(output.position, g_ProjectionMatrix);**/

	// Send the input color into the pixel shader.
	output.color = patch[0].color;

	return output;
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////

float4 ColorPixelShader(PixelInputType input) : SV_TARGET
{
	return input.color;
}