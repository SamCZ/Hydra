#include "Hydra/Import/TextureImporter.h"

#include "Hydra/Engine.h"
#include "Hydra/Render/Pipeline/DDS/DDSTextureLoader.h"

namespace Hydra
{
	using namespace Microsoft::WRL;

	NVRHI::TextureHandle TextureImporter::Import(const File& file)
	{
		IRendererInterface renderInterface = Engine::GetRenderInterface();
		
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