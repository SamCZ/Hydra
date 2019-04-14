#include "Hydra/HydraEngine.h"
#include "Hydra/Render/Pipeline/View/MainRenderView.h"
#include "Hydra/Render/Pipeline/View/UIRenderView.h"

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

		deviceManager->InitContext();
	}

	void HydraEngine::PrepareForEngineStart(DeviceCreationParameters& params)
	{
		Context->ScreenSize = Vector2i(params.Width, params.Height);
	}
}