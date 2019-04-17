#include "UIRenderView.h"
#include "Hydra/EngineContext.h"

#include "Hydra/HydraEngine.h"

#include "Hydra/Core/Random.h"

namespace Hydra
{
	static Vector2 Velocity;
	static Vector2 Acceleration;

	static Vector2 Position;

	static Random Rnd;

	void UIRenderView::OnCreated()
	{
		Position = (Vector2)Engine->GetContext()->ScreenSize * 0.5f;

		Velocity = Rnd.GetRandomUnitVector2() * 10.0f;
		Acceleration = Vector2(1.005f, 1.005f);
	}

	void UIRenderView::OnDestroy()
	{

	}

	void UIRenderView::OnRender(NVRHI::TextureHandle mainRenderTarget)
	{
		UIRenderer* renderer = Context->GetUIRenderer();

		

		renderer->DrawOval(Position.x, Position.y, 10, 10, ColorRGBA::Blue);

		renderer->DrawString("Ahoj", 10, 10, 26, ColorRGBA::Red);
	}

	void UIRenderView::OnTick(float Delta)
	{
		//;Velocity *= Acceleration;
		Position += Velocity;


		if (Position.x > 1280 || Position.x <= 0)
		{
			Velocity.x *= -1;
		}

		if (Position.y > 720 || Position.y <= 0)
		{
			Velocity.y *= -1;
		}
	}

	void UIRenderView::OnResize(uint32 width, uint32 height, uint32 sampleCount)
	{
		
	}
}
