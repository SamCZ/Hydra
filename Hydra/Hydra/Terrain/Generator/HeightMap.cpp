#include "Hydra/Terrain/Generator/HeightMap.h"
#include "Hydra/Engine.h"
#include "Hydra/Render/Graphics.h"

namespace Hydra
{
	HeightMap::~HeightMap()
	{
		delete[] Data;

		if (Texture)
		{
			Engine::GetRenderInterface()->destroyTexture(Texture);
		}
	}

	void HeightMap::InitalizeTexture()
	{
		if (!Texture)
		{
			NVRHI::TextureDesc textureDesc;
			textureDesc.width = Width;
			textureDesc.height = Height;
			textureDesc.mipLevels = 1;
			textureDesc.format = NVRHI::Format::R32_FLOAT;
			textureDesc.debugName = "HeightMap";
			Texture = Engine::GetRenderInterface()->createTexture(textureDesc, NULL);
		}
	}

	void HeightMap::UploadTextureData()
	{
		if (Texture)
		{
			Engine::GetRenderInterface()->writeTexture(Texture, 0, Data, Width * 4, 0);
			//Engine::GetRenderInterface()->generateMipmaps(Texture);
		}
	}
}