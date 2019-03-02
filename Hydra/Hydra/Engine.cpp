#include "Hydra/Engine.h"

namespace Hydra
{
	IRendererInterface Engine::_RenderInterface;
	SharedPtr<DeviceManager> Engine::_DeviceManager;
	InputManagerPtr Engine::_InputManager;
	Vector2 Engine::ScreenSize;

	void Engine::SetRenderInterface(IRendererInterface renderInterface)
	{
		_RenderInterface = renderInterface;
	}

	IRendererInterface Engine::GetRenderInterface()
	{
		return _RenderInterface;
	}

	void Engine::SetDeviceManager(SharedPtr<DeviceManager> deviceManager)
	{
		_DeviceManager = deviceManager;
	}

	SharedPtr<DeviceManager> Engine::GetDeviceManager()
	{
		return _DeviceManager;
	}

	void Engine::SetInputManager(InputManagerPtr inputManager)
	{
		_InputManager = inputManager;
	}

	InputManagerPtr Engine::GetInputManager()
	{
		return _InputManager;
	}
}