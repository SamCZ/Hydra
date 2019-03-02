#include "Assets/Shaders/RenderStage/Deffered.hlsli"

#pragma hydra vert:MainVS pixel:MainPS
#pragma hydra rs:Deffered

PS_Input OnMainVS(in VS_Input input, in PS_Input output)
{
	return output;
}

PS_Attributes OnMainPS(in PS_Input input, in PS_Attributes output)
{
	output.AlbedoMetallic = float4(1.0, 1.0, 1.0, 1.0);

	return output;
}