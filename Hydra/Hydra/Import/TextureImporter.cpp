#include "Hydra/Import/TextureImporter.h"

#define STB_IMAGE_IMPLEMENTATION

#include "Hydra/Engine.h"
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
		IRendererInterface renderInterface = Engine::GetRenderInterface();

		if (file.GetExtension() != "dds")
		{
			FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileType(file.GetPath().c_str());

			FIBITMAP* pBitmap = FreeImage_Load(imageFormat, file.GetPath().c_str(), TARGA_DEFAULT);
			if (pBitmap)
			{
				UINT width = FreeImage_GetWidth(pBitmap);
				UINT height = FreeImage_GetHeight(pBitmap);
				UINT bpp = FreeImage_GetBPP(pBitmap);

				NVRHI::Format::Enum format;
				FIBITMAP* newBitmap = NULL;

				switch (bpp)
				{
				case 8:
					format = NVRHI::Format::R8_UNORM;
					break;

				case 24:
					newBitmap = FreeImage_ConvertTo32Bits(pBitmap);
					FreeImage_Unload(pBitmap);
					pBitmap = newBitmap;
					format = NVRHI::Format::BGRA8_UNORM;
					break;

				case 32:
					format = NVRHI::Format::BGRA8_UNORM;
					break;

				default:
					return NULL;
				}

				NVRHI::TextureDesc textureDesc;
				textureDesc.width = width;
				textureDesc.height = height;
				textureDesc.mipLevels = getMipLevelsNum(width, height);
				textureDesc.format = format;
				textureDesc.debugName = file.GetName().c_str();
				NVRHI::TextureHandle texture = renderInterface->createTexture(textureDesc, NULL);

				if (texture)
				{
					for (UINT mipLevel = 0; mipLevel < textureDesc.mipLevels; mipLevel++)
					{
						UINT freeImagePitch = FreeImage_GetPitch(pBitmap);
						BYTE* bitmapData = FreeImage_GetBits(pBitmap);

						renderInterface->writeTexture(texture, mipLevel, bitmapData, freeImagePitch, 0);

						if (mipLevel < textureDesc.mipLevels - 1u)
						{
							width = __max(1, width >> 1);
							height = __max(1, height >> 1);
							newBitmap = FreeImage_Rescale(pBitmap, width, height, FILTER_BILINEAR);
							FreeImage_Unload(pBitmap);
							pBitmap = newBitmap;
						}
					}
				}

				FreeImage_Unload(pBitmap);

				return texture;
			}
		}

		/*
		if (file.GetExtension() == "png")
		{
			int x, y, bytesPerPixel;
			unsigned char* pixels = nullptr;

			pixels = stbi_load(file.GetPath().c_str(), &x, &y, &bytesPerPixel, 4);

			if (pixels == NULL)
			{
				std::cerr << "Unable to load texture(" << stbi_failure_reason() << "): " << file.GetPath() << std::endl;
				return nullptr;
			}

			NVRHI::TextureDesc gbufferDesc;
			gbufferDesc.width = x;
			gbufferDesc.height = y;
			gbufferDesc.isRenderTarget = false;
			gbufferDesc.useClearValue = false;
			gbufferDesc.sampleCount = 1;
			gbufferDesc.disableGPUsSync = true;

			gbufferDesc.mipLevels = 1;

			gbufferDesc.format = NVRHI::Format::RGBA8_UNORM;
			gbufferDesc.debugName = file.GetPath().c_str();

			NVRHI::TextureHandle handle = Engine::GetRenderInterface()->createTexture(gbufferDesc, NULL);

			Engine::GetRenderInterface()->writeTexture(handle, 0, pixels, x * 4, 0);
			Engine::GetRenderInterface()->generateMipmaps(handle);

			stbi_image_free(pixels);

			return handle;
		}

		if (file.GetExtension() == "hdr")
		{
			//stbi_set_flip_vertically_on_load(true);
			int width, height, nrComponents;
			float *data = stbi_loadf(file.GetPath().c_str(), &width, &height, &nrComponents, 4);
			//stbi_set_flip_vertically_on_load(false);

			if (data == NULL)
			{
				std::cerr << "Unable to load texture(" << stbi_failure_reason() << "): " << file.GetPath() << std::endl;
				return nullptr;
			}

			NVRHI::TextureDesc gbufferDesc;
			gbufferDesc.width = width;
			gbufferDesc.height = height;
			gbufferDesc.isRenderTarget = false;
			gbufferDesc.useClearValue = false;
			gbufferDesc.sampleCount = 1;
			gbufferDesc.disableGPUsSync = true;

			gbufferDesc.mipLevels = 1;

			gbufferDesc.format = NVRHI::Format::RGBA8_UNORM;
			gbufferDesc.debugName = file.GetPath().c_str();

			NVRHI::TextureHandle handle = Engine::GetRenderInterface()->createTexture(gbufferDesc, NULL);

			Engine::GetRenderInterface()->writeTexture(handle, 0, data, width * 4, 0);
			Engine::GetRenderInterface()->generateMipmaps(handle);

			//stbi_image_free(data);

			Log("TextureImporter::Import(" + file.GetPath() + ")", "Loaded.");

			return handle;
		}*/

		if (file.GetExtension() != "dds")
		{
			Log("TextureImporter::Import(" + file.GetPath() + ")", "Cannot load other texture type then dds !");
			return nullptr;
		}

		wchar_t wchTitle[256];
		MultiByteToWideChar(CP_ACP, 0, file.GetPath().c_str(), -1, wchTitle, 256);

		ComPtr<ID3D11Resource> resources[1];
		bool success = SUCCEEDED(DirectX::CreateDDSTextureFromFile(Engine::GetDeviceManager()->GetDevice(), wchTitle, &resources[0], NULL));

		if (!success)
		{
			LogError("TextureImporter::Import(" + file.GetPath() + ")", "Cannot be loaded.");
			return nullptr;
		}

		Log("TextureImporter::Import(" + file.GetPath() + ")", "Loaded.");

		return renderInterface->getHandleForTexture(resources[0].Get());
	}
}