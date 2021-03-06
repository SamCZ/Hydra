#include "GameViewWidget.h"

#include "Hydra/HydraEngine.h"
#include "Hydra/Render/Pipeline/View/MainRenderView.h"

#include "Hydra/Input/Windows/WindowsInputManager.h"
#include "Hydra/Render/Graphics.h"

NVRHI::TextureHandle tex;

GameViewWidget::GameViewWidget()
{

}

GameViewWidget::~GameViewWidget()
{

}

void GameViewWidget::Initialize(const FArguments& arguments)
{
	GEngine->GetContext()->ScreenSize = Vector2(500, 500);
	GEngine->GetContext()->SetInputManager(new WindowsInputManager(GEngine->GetContext()));

	//tex = GEngine->GetContext()->GetGraphics()->CreateRenderTarget("yo", NVRHI::Format::RGBA8_UNORM, 500, 500, NVRHI::Color(1, 1, 1, 1), 1);
}

int32 GameViewWidget::OnPaint(FPaintRenderQueueLayered & paintQueue, UIRenderer & renderer, int layerID)
{
	paintQueue.Add(layerID, [&]()
	{
		//renderer.DrawRect(0, 0, 600, 600, ColorRGBA::Blue);
		renderer.DrawImage(GEngine->GetContext()->GetGraphics()->GetRenderTarget("HGameView"), 0, 0, 500, 500);
	});

	return layerID + 1;
}
