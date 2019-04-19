#include "Hydra/EngineContext.h"

namespace Hydra
{
	EngineContext::EngineContext() : _RenderInterface(nullptr), _DeviceManager(nullptr), _InputManager(nullptr), _Graphics(nullptr), _UIRenderer(nullptr) : _AssetManager(nullptr)
	{

	}

	void EngineContext::SetRenderInterface(IRendererInterface renderInterface)
	{
		_RenderInterface = renderInterface;
	}

	IRendererInterface EngineContext::GetRenderInterface()
	{
		return _RenderInterface;
	}

	void EngineContext::SetDeviceManager(DeviceManager* deviceManager)
	{
		_DeviceManager = deviceManager;
	}

	DeviceManager* EngineContext::GetDeviceManager()
	{
		return _DeviceManager;
	}

	void EngineContext::SetInputManager(InputManager* inputManager)
	{
		_InputManager = inputManager;
	}

	InputManager* EngineContext::GetInputManager()
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

	void EngineContext::SetUIRenderer(UIRenderer* uiRenderer)
	{
		_UIRenderer = uiRenderer;
	}

	UIRenderer* EngineContext::GetUIRenderer()
	{
		return _UIRenderer;
	}

	void EngineContext::SetAssetManager(AssetManager* assetManager)
	{
		_AssetManager = assetManager;
	}

	AssetManager* EngineContext::GetAssetManager()
	{
		return _AssetManager;
	}
}