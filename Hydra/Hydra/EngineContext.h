#pragma once

#include "Hydra/Core/Log.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"
#include "Hydra/Render/Pipeline/DeviceManager11.h"
#include "Hydra/Core/SmartPointer.h"
#include "Hydra/Input/InputManager.h"

namespace Hydra
{
	typedef SharedPtr<NVRHI::IRendererInterface> IRendererInterface;

	class Graphics;

	class EngineContext
	{
	private:
		IRendererInterface _RenderInterface;
		SharedPtr<DeviceManager> _DeviceManager;
		InputManagerPtr _InputManager;
		Graphics* _Graphics;
	public:
		Vector2i ScreenSize;

		void SetRenderInterface(IRendererInterface renderInterface);
		IRendererInterface GetRenderInterface();

		void SetDeviceManager(SharedPtr<DeviceManager> deviceManager);
		SharedPtr<DeviceManager> GetDeviceManager();

		void SetInputManager(InputManagerPtr inputManager);
		InputManagerPtr GetInputManager();

		void SetGraphics(Graphics* graphics);
		Graphics* GetGraphics();
	};
}