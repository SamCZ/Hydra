#pragma once

#include "Hydra/Render/RenderStage.h"
#include "Hydra/Render/Shader.h"
#include "Hydra/Render/Graphics.h"

namespace Hydra
{
	struct alignas(16) GlobalConstants
	{
		alignas(16) Matrix4 g_ProjectionMatrix;
		alignas(16) Matrix4 g_ViewMatrix;
	};

	struct alignas(16) ModelConstants
	{
		alignas(16) Matrix4 g_ModelMatrix;
	};

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
		ShaderPtr _DefaultShader;
		ShaderPtr _CompositeShader;

		TexturePtr _BrdfLutTexture;
		SamplerPtr _BrdfLutSampler;

		TexturePtr _Albedo;
		TexturePtr _Normal;
		TexturePtr _Roughness;
		TexturePtr _Metallic;
		TexturePtr _AO;

		TexturePtr _IrrConv;
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