#pragma once

#include "Hydra/Core/Common.h"

namespace Hydra {

	class Renderer;
	class Spatial;

	class RenderManager
	{
	public:
		SharedPtr<Spatial> MainScene;

		List<SharedPtr<Renderer>> GetRenderersForStage(const String& stage);
	};

	DEFINE_PTR(RenderManager)
}