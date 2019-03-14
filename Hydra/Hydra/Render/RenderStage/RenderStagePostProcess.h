#pragma once

#include "Hydra/Render/RenderStage.h"
#include "Hydra/Render/Graphics.h"

namespace Hydra
{
	class RenderStagePostProcess : public RenderStage
	{
	public:
		virtual void Render(RenderManagerPtr rm) override;

		virtual void AllocateViewDependentResources(uint32 width, uint32 height, uint32 sampleCount) override;

		virtual String GetOutputName() override;
		virtual String GetDepthOutputName() override;

		virtual String GetName() override;
	};
}