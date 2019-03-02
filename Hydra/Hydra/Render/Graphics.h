#pragma once

#include "Hydra/Core/ColorRGBA.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/String.h"
#include "Hydra/Core/Function.h"

#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"
#include "Hydra/Render/Shader.h"

namespace Hydra
{
	enum PipelineStageBindingType : unsigned int
	{
		PSB_VERTEX      = 1 << 1,
		PSB_HULL        = 1 << 2,
		PSB_DOMAIN      = 1 << 3,
		PSB_GEOMETRY    = 1 << 4,
		PSB_PIXEL       = 1 << 5,
		PSB_SHADERS_NUM = 1 << 6,
		PSB_COMPUTE     = 1 << 7
	};

	typedef NVRHI::InputLayoutHandle InputLayoutPtr;
	typedef NVRHI::TextureHandle TexturePtr;
	typedef NVRHI::ConstantBufferHandle ConstantBufferPtr;
	typedef NVRHI::SamplerHandle SamplerPtr;

	typedef NVRHI::SamplerDesc::WrapMode WrapMode;

	struct ConstantBufferInfo
	{
		String Name;
		uint32_t DataSize;
		unsigned int SpecificBinding;
		int Slot;
		ConstantBufferPtr Handle;
	};

	class Graphics
	{
	private:
		static Map<String, ConstantBufferInfo> _ConstantBuffers;
		static Map<String, TexturePtr> _RenderViewTargets;
		static Map<String, InputLayoutPtr> _InputLayouts;
		static Map<String, SamplerPtr> _Samplers;

		static ShaderPtr _BlitShader;
	public:
		//Graphics();
		//~Graphics();

		static void Create();
		static void Destroy();

		static void Blit(TexturePtr pSource, TexturePtr pDest);
		static void Blit(const String& name, TexturePtr pDest);

		static void Composite(ShaderPtr shader, Function<void(NVRHI::DrawCallState&)> preRenderFunction, TexturePtr pDest);
		static void Composite(ShaderPtr shader, Function<void(NVRHI::DrawCallState&)> preRenderFunction, const String& outputName);

		static void SetShader(NVRHI::DrawCallState& state, ShaderPtr shader);

		static void SetClearFlags(NVRHI::DrawCallState& state, const ColorRGBA& color);

		static NVRHI::ConstantBufferHandle CreateConstantBuffer(uint32_t size, const String& mappedName = String_None, const unsigned int specificBinding = 0, int slot = -1);

		static void WriteConstantBufferData(NVRHI::ConstantBufferHandle handle, const void* data, uint32_t size);
		static void WriteConstantBufferData(          const String& mappedName, const void* data);
		static void WriteConstantBufferDataAndBind(NVRHI::DrawCallState& state, const String& mappedName, const void* data);

		static void BindConstantBuffer(NVRHI::PipelineStageBindings& ds, uint32_t slot, NVRHI::ConstantBufferHandle handle);
		static void BindConstantBuffer(NVRHI::DrawCallState& state, const String& mappedName, uint32_t slot = 0, bool slotOverride = false);

		static TexturePtr CreateRenderTarget(const String& name, const NVRHI::Format::Enum& format, UINT width, UINT height, const NVRHI::Color& clearColor, UINT sampleCount);
		static TexturePtr GetRenderTarget(const String& name);
		static void ReleaseRenderTarget(const String& name);
		static void BindRenderTarget(NVRHI::DrawCallState& state, const String& name, int index);

		static InputLayoutPtr CreateInputLayout(const String& name, const NVRHI::VertexAttributeDesc* d, uint32_t attributeCount, ShaderPtr shader);
		static InputLayoutPtr GetInputLayout(const String& name);

		static SamplerPtr CreateSampler(const String& name, const WrapMode& wrapX = WrapMode::WRAP_MODE_WRAP, const WrapMode& wrapY = WrapMode::WRAP_MODE_WRAP, const WrapMode& wrapZ = WrapMode::WRAP_MODE_WRAP, bool minFilter = true, bool magFilter = true, bool mipFilter = true, int anisotropy = 16);
		static SamplerPtr GetSampler(const String& name);
	};
}