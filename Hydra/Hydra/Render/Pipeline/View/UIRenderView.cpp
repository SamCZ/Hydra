#include "UIRenderView.h"
#include "Hydra/EngineContext.h"

namespace Hydra
{
	static float x = 0;

	void UIRenderView::OnCreated()
	{

	}

	void UIRenderView::OnDestroy()
	{

	}

	void UIRenderView::OnRender(NVRHI::TextureHandle mainRenderTarget)
	{
		UIRenderer* renderer = Context->GetUIRenderer();

		

		renderer->DrawOval(x, x, 10, 10, Colors::Blue);

		renderer->DrawString("Ahoj", 10, 10, 26, Colors::Blue);
	}

	void UIRenderView::OnTick(float Delta)
	{
		x += 1;
	}

	void UIRenderView::OnResize(uint32 width, uint32 height, uint32 sampleCount)
	{

	}
}
