#pragma once

#include "Hydra/Render/RenderManager.h"
#include "Hydra/Render/TextureLayoutDef.h"

namespace Hydra
{
	class RenderStage
	{
	protected:
		TextureLayoutDefPtr TextureLayout;
	public:

		void SetTextureLayoutDef(TextureLayoutDefPtr layout);

		virtual void Render(RenderManagerPtr rm) = 0;

		virtual void AllocateViewDependentResources(uint32 width, uint32 height, uint32 sampleCount) = 0;

		virtual String GetOutputName() = 0;

		virtual String GetDepthOutputName() = 0;

		virtual String GetName() = 0;
	};
}