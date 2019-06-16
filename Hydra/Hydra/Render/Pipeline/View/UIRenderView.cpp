#include "UIRenderView.h"
#include "Hydra/EngineContext.h"

#include "Hydra/HydraEngine.h"

#include "Hydra/Core/Random.h"
#include "Hydra/Core/Sort.h"

#include "Hydra/Framework/World.h"
#include "Hydra/Framework/Actor.h"

#include "Hydra/Render/Graphics.h"

void UIRenderView::OnCreated()
{
	//Context->GetGraphics()->CreateRenderTarget("UI", NVRHI::Format::RGBA8_UNORM, 1280, 720, NVRHI::Color(1.0), 1);


}

void UIRenderView::OnDestroy()
{

}

int HudRenderCompareFnc(AActor* left, AActor* right)
{
	if (left->Layer == right->Layer) return 0;

	return left->Layer > right->Layer ? 1 : -1;
}

void UIRenderView::OnRender(NVRHI::TextureHandle mainRenderTarget)
{
	//(void)mainRenderTarget;

	UIRenderer* renderer = Context->GetUIRenderer();

	//renderer->SetRenderTarget(Context->GetGraphics()->GetRenderTarget("UI"));

	List<AActor*> actors;

	for (AActor* actor : Engine->GetWorld()->GetActors())
	{
		if (actor->IsActive)
		{
			actors.push_back(actor);
		}
	}

	MergeSort<AActor*>(actors, HudRenderCompareFnc);

	renderer->Begin(Context->ScreenSize);

	for (AActor* actor : actors)
	{
		actor->OnHud(renderer);
	}

	ColorRGBA bigLineColor = MakeRGB(100, 100, 100);
	ColorRGBA smallLineColor = MakeRGB(80, 80, 80);

	float GRID_SZ = 40.0f;
	float _scrollX = 0;
	float _scrollY = 0;

	int w = Context->ScreenSize.x;
	int h = Context->ScreenSize.y;

	for (float x = fmodf(_scrollX, GRID_SZ); x < w; x += GRID_SZ)
	{
		renderer->RB_RenderLine(x, 0, x, h, 1.0f, smallLineColor);
	}

	for (float y = fmodf(_scrollY, GRID_SZ); y < h; y += GRID_SZ)
	{
		renderer->RB_RenderLine(0, y, w, y, 1.0f, smallLineColor);
	}

	renderer->RB_RenderBlock("Multiply", 100, 100, 200, 125, true);

	renderer->RB_RenderSpline(100, 100, Context->GetInputManager()->GetCursorPos().x, Context->GetInputManager()->GetCursorPos().y, 4, 1);
	
	renderer->DrawImage(Context->GetGraphics()->GetRenderTarget("HGameView"), 300, 300, 640, 350);

	renderer->End();

	//Context->GetGraphics()->Blit("UI", mainRenderTarget);
}

void UIRenderView::OnTick(float Delta)
{
	
}

void UIRenderView::OnResize(uint32 width, uint32 height, uint32 sampleCount)
{

}