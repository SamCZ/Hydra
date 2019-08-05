#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/App/Render/PaintRenderQueueLayered.h"
#include "Hydra/Render/UI/UIRenderer.h"

class UIWindow;

class HYDRA_API UIWidget : public SharedFromThis<UIWidget>
{
protected:
	SharedPtr<UIWidget> Parent;
	SharedPtr<UIWindow> ParentWindow;
public:
	UIWidget();
	virtual ~UIWidget();

	virtual int32 OnPaint(FPaintRenderQueueLayered& paintQueue, UIRenderer& renderer, int layerID);

	void SetParent(SharedPtr<UIWidget> widget);
	void SetParentWindow(SharedPtr<UIWindow> window);

	SharedPtr<UIWidget>& GetParent();
	SharedPtr<UIWindow>& GetParentWindow();
};