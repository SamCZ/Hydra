#include "Assets/Shaders/RenderStage/Deffered.hlsli"

#pragma hydra vert:MainVS pixel:MainPS
#pragma hydra rs:Deffered

PS_Input OnMainVS(in VS_Input input, in PS_Input output)
{
	return output;
}

PBROutput OnMainPS(in PS_Input input)
{
	PBROutput output;

	output.Albedo = float3(1.0, 0.5, 0.8);
	output.Normal = input.normal;
	output.Metallic = 0.0;
	output.Roughness = 1.0;
	output.AO = 0.0;
	output.Emission = float3(0.0, 0.0, 0.0);

	return output;
}