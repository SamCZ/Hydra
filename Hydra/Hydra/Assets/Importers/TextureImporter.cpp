#include "TextureImporter.h"

#include <wrl.h>

#include "Hydra/Render/Pipeline/Windows/DDS/DDSTextureLoader.h"
#include "FreeImage/FreeImage.h"

#include "Hydra/EngineContext.h"

using namespace Microsoft::WRL;

UINT getMipLevelsNum(UINT width, UINT height)
{
	UINT size = __max(width, height);
	UINT levelsNum = (UINT)(logf((float)size) / logf(2.0f)) + 1;

	return levelsNum;
}

TextureImporter::TextureImporter(EngineContext * context) : _Context(context)
{
}

bool TextureImporter::Import(Blob & dataBlob, const AssetImportOptions& options, List<HAsset*>& out_Assets)
{
	//TODO: Get file type
	//TODO: complete image loading

	NVRHI::IRendererInterface* renderInterface = _Context->GetRenderInterface();

	String fileType = "png";

	if (fileType != "dds")
	{
		FIMEMORY* hmem = FreeImage_OpenMemory((BYTE*)dataBlob.GetData(), dataBlob.GetDataSize());

		FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileTypeFromMemory(hmem, 0);

		FIBITMAP* pBitmap = FreeImage_LoadFromMemory(imageFormat, hmem);
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

			Log("Bpp", ToString(bpp));

			NVRHI::TextureDesc textureDesc;
			textureDesc.width = width;
			textureDesc.height = height;
			textureDesc.mipLevels = getMipLevelsNum(width, height);
			textureDesc.format = format;
			textureDesc.debugName = "Loaded texture";
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
			FreeImage_CloseMemory(hmem);

			out_Assets = { texture };

			return true;
		}

		FreeImage_CloseMemory(hmem);
	}

	ComPtr<ID3D11Resource> resources[1];

	bool success = SUCCEEDED(DirectX::CreateDDSTextureFromMemory((ID3D11Device*)nullptr, (const uint8_t*)dataBlob.GetData(), dataBlob.GetDataSize(), &resources[0], NULL));

	if (success)
	{

	}

	return success;
}
