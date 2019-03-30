#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

namespace Hydra
{
	struct HeightMap
	{
		int Width;
		int Height;
		float* Data;
		NVRHI::TextureHandle Texture;

		~HeightMap();

		void InitalizeTexture();
		void UploadTextureData();
	};
}