#include "Hydra/Import/TextureImporter.h"

#define STB_IMAGE_IMPLEMENTATION

#include "Hydra/Engine.h"
#include "Hydra/Render/Pipeline/DDS/DDSTextureLoader.h"
#include "NanoVG/stb_image.h"

namespace Hydra
{
	using namespace Microsoft::WRL;

	NVRHI::TextureHandle TextureImporter::Import(const File& file)
	{
		IRendererInterface renderInterface = Engine::GetRenderInterface();
		
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

			return handle;
		}

		if (file.GetExtension() != "dds")
		{
			Log("TextureImporter::Import(" + file.GetPath() + ")", "Cannot load other texture type then dds !");
			return nullptr;
		}

		wchar_t wchTitle[256];
		MultiByteToWideChar(CP_ACP, 0, file.GetPath().c_str(), -1, wchTitle, 256);

		ComPtr<ID3D11Resource> resources[1];
		int success = SUCCEEDED(DirectX::CreateDDSTextureFromFile(Engine::GetDeviceManager()->GetDevice(), wchTitle, &resources[0], NULL));

		return renderInterface->getHandleForTexture(resources[0].Get());
	}
}