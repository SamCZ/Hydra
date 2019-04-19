#include "Hydra/HydraEngine.h"
#include "Hydra/Render/Pipeline/View/MainRenderView.h"
#include "Hydra/Render/Pipeline/View/UIRenderView.h"

#include "Hydra/Framework/World.h"

namespace Hydra
{

	void HydraEngine::Start()
	{
		Context = new EngineContext();

		DeviceManager* deviceManager = DeviceManager::CreateDeviceManagerForPlatform();
		Context->SetDeviceManager(deviceManager);

		deviceManager->OnPrepareDeviceContext += EVENT_ARGS(HydraEngine, PrepareForEngineStart, DeviceCreationParameters&);

		deviceManager->AddVisualController(new MainRenderView(Context, this));
		deviceManager->AddVisualController(new UIRenderView(Context, this));

		World = new FWorld(Context);

		AssetManager* assetManager = new AssetManager();
		Context->SetAssetManager(assetManager);
		InitializeAssetManager(assetManager);

		deviceManager->InitContext();
	}

	void HydraEngine::PrepareForEngineStart(DeviceCreationParameters& params)
	{
		Context->ScreenSize = Vector2i(params.Width, params.Height);
	}

	void HydraEngine::InitializeAssetManager(AssetManager* assetManager)
	{

	}

	FWorld* HydraEngine::GetWorld() const
	{
		return World;
	}

	EngineContext * HydraEngine::GetContext() const
	{
		return Context;
	}
}