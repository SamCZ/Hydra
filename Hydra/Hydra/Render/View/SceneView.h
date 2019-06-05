#pragma once

#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

class FSceneView
{
public:
	NVRHI::TextureHandle RenderTexture;

	int Width;
	int Height;
};