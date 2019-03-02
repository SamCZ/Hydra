#pragma once

#include "Hydra/Core/Log.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"
#include "Hydra/Render/Pipeline/DeviceManager11.h"
#include "Hydra/Core/SmartPointer.h"
#include "Hydra/Input/InputManager.h"

namespace Hydra
{
	typedef SharedPtr<NVRHI::IRendererInterface> IRendererInterface;

	class Engine
	{
	private:
		static IRendererInterface _RenderInterface;
		static SharedPtr<DeviceManager> _DeviceManager;
		static InputManagerPtr _InputManager;

	public:
		static Vector2 ScreenSize;

		static void SetRenderInterface(IRendererInterface renderInterface);
		static IRendererInterface GetRenderInterface();

		static void SetDeviceManager(SharedPtr<DeviceManager> deviceManager);
		static SharedPtr<DeviceManager> GetDeviceManager();

		static void SetInputManager(InputManagerPtr inputManager);
		static InputManagerPtr GetInputManager();
	};
}