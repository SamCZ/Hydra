#pragma once

#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"
#include "Hydra/Core/File.h"

namespace Hydra
{
	class TextureImporter
	{
	public:
		static NVRHI::TextureHandle Import(const File& file);
	};
}