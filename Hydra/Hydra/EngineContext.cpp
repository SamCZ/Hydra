#include "Hydra/EngineContext.h"

namespace Hydra
{
	void EngineContext::SetRenderInterface(IRendererInterface renderInterface)
	{
		_RenderInterface = renderInterface;
	}

	IRendererInterface EngineContext::GetRenderInterface()
	{
		return _RenderInterface;
	}

	void EngineContext::SetDeviceManager(SharedPtr<DeviceManager> deviceManager)
	{
		_DeviceManager = deviceManager;
	}

	SharedPtr<DeviceManager> EngineContext::GetDeviceManager()
	{
		return _DeviceManager;
	}

	void EngineContext::SetInputManager(InputManagerPtr inputManager)
	{
		_InputManager = inputManager;
	}

	InputManagerPtr EngineContext::GetInputManager()
	{
		return _InputManager;
	}

	void EngineContext::SetGraphics(Graphics * graphics)
	{
		_Graphics = graphics;
	}

	Graphics* EngineContext::GetGraphics()
	{
		return _Graphics;
	}
}