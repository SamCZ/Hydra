#pragma once

#include "Hydra/App/UI/UI.h"
#include "Hydra/App/UI/UIWidget.h"

class MainRenderView;

class HYDRA_API GameViewWidget : public UIWidget
{
public:
	UI_BEGIN_ARGS(GameViewWidget)
	{

	}

	UI_END_ARGS;


private:
	MainRenderView* GameRenderView;
public:
	GameViewWidget();
	virtual ~GameViewWidget();

	void Initialize(const FArguments& arguments);

	virtual int32 OnPaint(FPaintRenderQueueLayered& paintQueue, UIRenderer& renderer, int layerID) override;
};