#include "Hydra/Terrain/Generator/HeightMap.h"
#include "Hydra/EngineContext.h"
#include "Hydra/Render/Graphics.h"

namespace Hydra
{
	HeightMap::~HeightMap()
	{
		delete[] Data;

		if (Texture)
		{
			_Context->GetRenderInterface()->destroyTexture(Texture);
		}
	}

	void HeightMap::InitalizeTexture(EngineContext* context, bool isUAV)
	{
		_Context = context;

		if (!Texture)
		{
			NVRHI::TextureDesc textureDesc;
			textureDesc.width = Width;
			textureDesc.height = Height;
			textureDesc.mipLevels = 1;
			textureDesc.format = NVRHI::Format::R32_FLOAT;
			textureDesc.debugName = "HeightMap";

			textureDesc.isUAV = isUAV;

			Texture = _Context->GetRenderInterface()->createTexture(textureDesc, NULL);
		}
	}

	void HeightMap::UploadTextureData()
	{
		if (Texture)
		{
			_Context->GetRenderInterface()->writeTexture(Texture, 0, Data, Width * 4, 0);
			//Engine::GetRenderInterface()->generateMipmaps(Texture);
		}
	}
}