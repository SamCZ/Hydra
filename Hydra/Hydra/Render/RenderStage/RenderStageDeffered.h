#pragma once

#include "Hydra/Render/RenderStage.h"
#include "Hydra/Render/Graphics.h"

namespace Hydra
{
	class Renderer;
	class Light;

	class RenderStageDeffered : public RenderStage
	{
	private:
		InputLayoutPtr _InputLayout;
		MaterialPtr _DefaultMaterial;
		MaterialPtr _ShadowMaterial;
		MaterialPtr _CompositeMaterial;

		MaterialPtr _PostEmissionPreMaterial;
		MaterialPtr _PostEmissionMaterial;

		MaterialPtr _PostSSAOMaterial;

		MaterialPtr _MultMaterial;

		TexturePtr _BrdfLutTexture;
		SamplerPtr _BrdfLutSampler;
	public:
		RenderStageDeffered();
		~RenderStageDeffered();

		void RenderLights(List<SharedPtr<Renderer>>& renderers, List<SharedPtr<Light>>& lights);

		virtual void Render(RenderManagerPtr rm) override;

		virtual void AllocateViewDependentResources(uint32 width, uint32 height, uint32 sampleCount) override;

		virtual String GetOutputName() override;
		virtual String GetDepthOutputName() override;

		virtual String GetName() override;
	};

	DEFINE_PTR(RenderStageDeffered)
}