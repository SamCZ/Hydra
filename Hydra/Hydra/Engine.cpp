#include "Hydra/Engine.h"

namespace Hydra
{
	IRendererInterface Engine::_RenderInterface;
	SharedPtr<DeviceManager> Engine::_DeviceManager;

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
}