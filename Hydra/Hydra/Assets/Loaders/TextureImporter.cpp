#include "Hydra/Assets/Loaders/TextureImporter.h"

#define STB_IMAGE_IMPLEMENTATION

#include "Hydra/EngineContext.h"
#include "Hydra/Render/Pipeline/DDS/DDSTextureLoader.h"

#include "FreeImage/FreeImage.h"

namespace Hydra
{
	using namespace Microsoft::WRL;

	UINT getMipLevelsNum(UINT width, UINT height)
	{
		UINT size = __max(width, height);
		UINT levelsNum = (UINT)(logf((float)size) / logf(2.0f)) + 1;

		return levelsNum;
	}

	NVRHI::TextureHandle TextureImporter::Import(const File& file)
	{
		return nullptr;
	}
}