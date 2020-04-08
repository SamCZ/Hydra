#pragma once

#include "Hydra/Core/SmartPointer.h"

class UIWindow;

class FWindowRender
{
public:

	virtual void CreateViewport(SharedPtr<UIWindow>& window) = 0;

	virtual void ResizeViewPort(SharedPtr<UIWindow>& window, int newWidth, int newHeight) = 0;

	virtual void RenderWindows() = 0;

	virtual void OnWindowDestroy(const SharedPtr<UIWindow>& window) = 0;
};