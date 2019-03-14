#pragma once

#include "Hydra/Render/RenderStage.h"
#include "Hydra/Render/Graphics.h"

namespace Hydra
{
	struct alignas(16) SingleFloatConstant
	{
		alignas(16) float Float;
	};

	struct alignas(16) Float3Constant
	{
		alignas(16) Vector3 Vector;
	};

	class RenderStageDeffered : public RenderStage
	{
	private:
		InputLayoutPtr _InputLayout;
		TechniquePtr _DefaultShader;
		TechniquePtr _CompositeShader;

		TechniquePtr _PostEmissionPreShader;
		TechniquePtr _PostEmissionShader;

		TechniquePtr _PostSSAOShader;

		TechniquePtr _MultShader;

		TexturePtr _BrdfLutTexture;
		SamplerPtr _BrdfLutSampler;
	public:
		RenderStageDeffered();
		~RenderStageDeffered();

		virtual void Render(RenderManagerPtr rm) override;

		virtual void AllocateViewDependentResources(uint32 width, uint32 height, uint32 sampleCount) override;

		virtual String GetOutputName() override;
		virtual String GetDepthOutputName() override;

		virtual String GetName() override;
	};

	DEFINE_PTR(RenderStageDeffered)
}