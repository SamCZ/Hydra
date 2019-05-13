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
	
	renderer->End();

	//Context->GetGraphics()->Blit("UI", mainRenderTarget);
}

void UIRenderView::OnTick(float Delta)
{
	
}

void UIRenderView::OnResize(uint32 width, uint32 height, uint32 sampleCount)
{

}