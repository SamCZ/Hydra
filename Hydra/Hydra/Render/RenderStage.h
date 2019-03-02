#pragma once

#include "Hydra/Render/RenderManager.h"

namespace Hydra
{
	class RenderStage
	{
	public:

		virtual void Render(RenderManagerPtr rm) = 0;

		virtual void AllocateViewDependentResources(uint32 width, uint32 height, uint32 sampleCount) = 0;

		virtual String GetOutputName() = 0;

		virtual String GetDepthOutputName() = 0;

		virtual String GetName() = 0;
	};
}