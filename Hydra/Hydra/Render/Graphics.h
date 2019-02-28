#pragma once

#include "Hydra/Core/ColorRGBA.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"
#include "Hydra/Render/Shader.h"

namespace Hydra
{
	class Graphics
	{
	private:

	public:
		//Graphics();
		//~Graphics();

		//void Blit(NVRHI::TextureHandle pSource, NVRHI::TextureHandle pDest);

		static void SetShader(NVRHI::DrawCallState& state, ShaderPtr shader);

		static void SetClearFlags(NVRHI::DrawCallState& state, const ColorRGBA& color);
	};
}