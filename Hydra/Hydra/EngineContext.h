#pragma once

#include "Hydra/Core/Log.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"
#include "Hydra/Render/Pipeline/Windows/DX11/DeviceManager11.h"
#include "Hydra/Core/SmartPointer.h"
#include "Hydra/Input/InputManager.h"
#include "Hydra/Render/UI/UIRenderer.h"

namespace Hydra
{
	typedef NVRHI::IRendererInterface* IRendererInterface;

	class Graphics;

	class HYDRA_API EngineContext
	{
	private:
		IRendererInterface _RenderInterface;
		DeviceManager* _DeviceManager;
		InputManager* _InputManager;
		Graphics* _Graphics;
		UIRenderer* _UIRenderer;
	public:
		Vector2i ScreenSize;

	public:
		EngineContext();

		void SetRenderInterface(IRendererInterface renderInterface);
		IRendererInterface GetRenderInterface();

		void SetDeviceManager(DeviceManager* deviceManager);
		DeviceManager* GetDeviceManager();

		void SetInputManager(InputManager* inputManager);
		InputManager* GetInputManager();

		void SetGraphics(Graphics* graphics);
		Graphics* GetGraphics();

		void SetUIRenderer(UIRenderer* uiRenderer);
		UIRenderer* GetUIRenderer();
	};
}