#pragma once

#include "Hydra/Core/ColorRGBA.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/String.h"

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

	struct ConstantBufferInfo
	{
		String Name;
		uint32_t DataSize;
		unsigned int SpecificBinding;
		uint32_t Slot;
		NVRHI::ConstantBufferHandle Handle;
	};

	class Graphics
	{
	private:
		static Map<String, ConstantBufferInfo> _ConstantBuffers;
		static Map<String, NVRHI::TextureHandle> _RenderViewTargets;
	public:
		//Graphics();
		//~Graphics();

		//void Blit(NVRHI::TextureHandle pSource, NVRHI::TextureHandle pDest);

		static void Destroy();

		static void SetShader(NVRHI::DrawCallState& state, ShaderPtr shader);

		static void SetClearFlags(NVRHI::DrawCallState& state, const ColorRGBA& color);

		static NVRHI::ConstantBufferHandle CreateConstantBuffer(uint32_t size, const String& mappedName = String_None, const unsigned int specificBinding = 0, uint32_t slot = -1);

		static void WriteConstantBufferData(NVRHI::ConstantBufferHandle handle, const void* data, uint32_t size);
		static void WriteConstantBufferData(          const String& mappedName, const void* data);
		static void WriteConstantBufferDataAndBind(NVRHI::DrawCallState& state, const String& mappedName, const void* data);

		static void BindConstantBuffer(NVRHI::PipelineStageBindings& ds, uint32_t slot, NVRHI::ConstantBufferHandle handle);
		static void BindConstantBuffer(NVRHI::DrawCallState& state, const String& mappedName, uint32_t slot = 0, bool slotOverride = false);

		static NVRHI::TextureHandle CreateRenderTarget(const String& name, const NVRHI::Format::Enum& format, UINT width, UINT height, const NVRHI::Color& clearColor, UINT sampleCount);
		static NVRHI::TextureHandle GetRenderTarget(const String& name);
	};
}