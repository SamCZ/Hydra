#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Core/Delegate.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"
#include "Hydra/Render/Pipeline/DeviceCreationParameters.h"

namespace Hydra
{
	class HYDRA_API EngineContext;

	class HYDRA_API IVisualController
	{
	public:
		EngineContext* Context;
	public:
		IVisualController(EngineContext* context) : Context(context) {}

		virtual void OnCreated() = 0;
		virtual void OnDestroy() = 0;

		virtual void OnRender(NVRHI::TextureHandle mainRenderTarget) = 0;
		virtual void OnTick(float Delta) = 0;
		virtual void OnResize(uint32 width, uint32 height, uint32 sampleCount) = 0;
	};

	class HYDRA_API DeviceManager
	{
	private:

	public:
		DelegateEvent<void, DeviceCreationParameters&> OnPrepareDeviceContext;

		virtual void InitContext() = 0;
		virtual void AddVisualController(IVisualController* view) = 0;
	public:

		static DeviceManager* CreateDeviceManagerForPlatform();
		static NVRHI::IRendererInterface* CreateRenderInterfaceForPlatform(DeviceManager* deviceManager);
	};
}