#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

namespace Hydra
{
	class EngineContext;

	struct HeightMap
	{
		EngineContext* _Context;

		int Width;
		int Height;
		float* Data;
		NVRHI::TextureHandle Texture;

		~HeightMap();

		void InitalizeTexture(EngineContext* context, bool isUAV = false);
		void UploadTextureData();
	};
}