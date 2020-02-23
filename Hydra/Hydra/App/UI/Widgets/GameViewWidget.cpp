#include "GameViewWidget.h"

#include "Hydra/App/UI/UIWindow.h"

#include "Hydra/HydraEngine.h"
#include "Hydra/Render/Pipeline/View/MainRenderView.h"

#include "Hydra/Render/Graphics.h"

GameViewWidget::GameViewWidget()
{

}

GameViewWidget::~GameViewWidget()
{

}

void GameViewWidget::Initialize(const FArguments& arguments)
{
	//tex = GEngine->GetContext()->GetGraphics()->CreateRenderTarget("yo", NVRHI::Format::RGBA8_UNORM, 500, 500, NVRHI::Color(1, 1, 1, 1), 1);
}

int32 GameViewWidget::OnPaint(FPaintRenderQueueLayered & paintQueue, UIRenderer & renderer, int layerID)
{
	SharedPtr<UIWindow> window = GetParentWindow();

	GEngine->GetContext()->GetInputManager()->Update(window);


	paintQueue.Add(layerID, [&]()
	{
		//renderer.DrawRect(0, 0, 600, 600, ColorRGBA::Blue);
		renderer.DrawImage(GEngine->GetContext()->GetGraphics()->GetRenderTarget("HGameView"), 0, 0, GEngine->GetContext()->ScreenSize.x, GEngine->GetContext()->ScreenSize.y);
	});

	return layerID + 1;
}
