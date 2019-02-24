#pragma once

#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

namespace Hydra
{
	class Graphics
	{
	private:

	public:
		Graphics();
		~Graphics();

		void Blit(NVRHI::TextureHandle pSource, NVRHI::TextureHandle pDest);
	};
}