#include "UIRenderView.h"
#include "Hydra/EngineContext.h"

#include "Hydra/HydraEngine.h"

#include "Hydra/Core/Random.h"

#include "Hydra/Framework/World.h"
#include "Hydra/Framework/Actor.h"

void UIRenderView::OnCreated()
{

}

void UIRenderView::OnDestroy()
{

}

void UIRenderView::OnRender(NVRHI::TextureHandle mainRenderTarget)
{
	UIRenderer* renderer = Context->GetUIRenderer();

	for (AActor* actor : Engine->GetWorld()->GetActors())
	{
		if (actor->IsActive)
		{
			Vector3 location = actor->RootComponent->Location;

			renderer->DrawOval(location.x, location.y, 10, 10, ColorRGBA::Blue);
		}
	}

	renderer->DrawString("Ahoj", 10, 10, 26, ColorRGBA::Red);
}

void UIRenderView::OnTick(float Delta)
{
	
}

void UIRenderView::OnResize(uint32 width, uint32 height, uint32 sampleCount)
{

}