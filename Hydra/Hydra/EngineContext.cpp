#include "Hydra/EngineContext.h"

EngineContext::EngineContext() : _RenderInterface(nullptr), _RenderManager(nullptr), _InputManager(nullptr), _Graphics(nullptr), _UIRenderer(nullptr), _AssetManager(nullptr)
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

void EngineContext::SetRenderManager(RenderManager* renderManager)
{
	_RenderManager = renderManager;
}

RenderManager* EngineContext::GetRenderManager()
{
	return _RenderManager;
}

void EngineContext::SetInputManager(InputManager* inputManager)
{
	_InputManager = inputManager;
}

InputManager* EngineContext::GetInputManager()
{
	return _InputManager;
}

void EngineContext::SetGraphics(FGraphics * graphics)
{
	_Graphics = graphics;
}

FGraphics* EngineContext::GetGraphics()
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