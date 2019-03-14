#pragma once

#include "Hydra/Render/RenderStage.h"
#include "Hydra/Render/Graphics.h"

namespace Hydra
{
	class PostEffect
	{
	private:

	public:

		virtual void Render(RenderManagerPtr rm) = 0;


	};
}