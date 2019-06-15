#pragma once

#include "Hydra/Core/Log.h"
#include "Hydra/Core/SmartPointer.h"
#include "Hydra/Render/RenderManager.h"
#include "Hydra/Render/UI/UIRenderer.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"
#include "Hydra/Render/Pipeline/Windows/DX11/DeviceManager11.h"

#include "Hydra/Input/InputManager.h"
#include "Hydra/Assets/AssetManager.h"

typedef NVRHI::IRendererInterface* IRendererInterface;

class FGraphics;

class HYDRA_API EngineContext
{
private:
	IRendererInterface _RenderInterface;
	RenderManager* _RenderManager;
	DeviceManager* _DeviceManager;
	InputManager* _InputManager;
	FGraphics* _Graphics;
	UIRenderer* _UIRenderer;
	AssetManager* _AssetManager;
public:
	Vector2i ScreenSize;

public:
	EngineContext();

	void SetRenderInterface(IRendererInterface renderInterface);
	IRendererInterface GetRenderInterface();

	void SetRenderManager(RenderManager* renderManager);
	RenderManager* GetRenderManager();

	void SetDeviceManager(DeviceManager* deviceManager);
	DeviceManager* GetDeviceManager();

	void SetInputManager(InputManager* inputManager);
	InputManager* GetInputManager();

	void SetGraphics(FGraphics* graphics);
	FGraphics* GetGraphics();

	void SetUIRenderer(UIRenderer* uiRenderer);
	UIRenderer* GetUIRenderer();

	void SetAssetManager(AssetManager* assetManager);
	AssetManager* GetAssetManager();
};