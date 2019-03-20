#pragma once

#include "Hydra/Core/ColorRGBA.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/String.h"
#include "Hydra/Core/Function.h"

#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"
#include "Hydra/Render/Material.h"

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

	#define CmpFnc Function<void(NVRHI::DrawCallState&, int, int)>

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

		static MaterialPtr _BlitMaterial;
		static MaterialPtr _BlurMaterial;
	public:
		//Graphics();
		//~Graphics();

		static void Create();
		static void Destroy();

		static void AllocateViewDependentResources(uint32 width, uint32 height, uint32 sampleCount);

		static void Blit(TexturePtr pSource, TexturePtr pDest);
		static void Blit(const String& name, TexturePtr pDest);
		static void Blit(const String& pSource, const String& pDest);

		static void BlurTexture(TexturePtr pSource, TexturePtr pDest);
		static void BlurTexture(const String pSource, const String pDest);

		static void Composite(MaterialPtr mateiral, Function<void(NVRHI::DrawCallState&)> preRenderFunction, TexturePtr pDest);
		static void Composite(MaterialPtr mateiral, Function<void(NVRHI::DrawCallState&)> preRenderFunction, const String& outputName);
		static void Composite(MaterialPtr mateiral, TexturePtr slot0Texture, TexturePtr pDest);
		static void Composite(MaterialPtr mateiral, const String& slot0Texture, const String& pDest);

		static void RenderCubeMap(MaterialPtr mateiral, InputLayoutPtr inputLayout, const Vector2& viewPort, Function<void(NVRHI::DrawCallState&, int, int)> preRenderFunction, TexturePtr pDest);
		static void RenderCubeMap(MaterialPtr mateiral, const String& inputLayout, const Vector2& viewPort, Function<void(NVRHI::DrawCallState&, int, int)> preRenderFunction, const String& outputName);

		static void SetMaterialShaders(NVRHI::DrawCallState& state, MaterialPtr mateiral);
		static void ApplyMaterialParameters(NVRHI::DrawCallState& state, MaterialPtr mateiral);

		static void SetClearFlags(NVRHI::DrawCallState& state, const ColorRGBA& color);

		static NVRHI::ConstantBufferHandle CreateConstantBuffer(uint32_t size, const String& mappedName = String_None, const unsigned int specificBinding = 0, int slot = -1);

		static void WriteConstantBufferData(NVRHI::ConstantBufferHandle handle, const void* data, uint32_t size);
		static void WriteConstantBufferData(          const String& mappedName, const void* data);
		static void WriteConstantBufferDataAndBind(NVRHI::DrawCallState& state, const String& mappedName, const void* data);

		static void BindConstantBuffer(NVRHI::PipelineStageBindings& ds, uint32_t slot, NVRHI::ConstantBufferHandle handle);
		static void BindConstantBuffer(NVRHI::DrawCallState& state, const String& mappedName, uint32_t slot = 0, bool slotOverride = false);

		static ConstantBufferPtr GetConstantBuffer(const String& mappedName);

		static TexturePtr CreateRenderTarget(const String& name, const NVRHI::Format::Enum& format, UINT width, UINT height, const NVRHI::Color& clearColor, UINT sampleCount);
		static TexturePtr CreateRenderTarget2DArray(const String& name, const NVRHI::Format::Enum& format, UINT width, UINT height, int mipCount, int arrSize);
		static TexturePtr CreateRenderTargetCubeMap(const String& name, const NVRHI::Format::Enum& format, UINT width, UINT height, const NVRHI::Color& clearColor, int mipLevels = 1);

		static TexturePtr GetRenderTarget(const String& name);
		static void ReleaseRenderTarget(const String& name);
		static void BindRenderTarget(NVRHI::DrawCallState& state, const String& name, int index);

		static InputLayoutPtr CreateInputLayout(const String& name, const NVRHI::VertexAttributeDesc* d, uint32_t attributeCount, MaterialPtr material);
		static InputLayoutPtr GetInputLayout(const String& name);

		static SamplerPtr CreateSampler(const String& name, const WrapMode& wrapX = WrapMode::WRAP_MODE_WRAP, const WrapMode& wrapY = WrapMode::WRAP_MODE_WRAP, const WrapMode& wrapZ = WrapMode::WRAP_MODE_WRAP, bool minFilter = true, bool magFilter = true, bool mipFilter = true, int anisotropy = 16);
		static SamplerPtr CreateShadowCompareSampler(const String& name);
		static SamplerPtr GetSampler(const String& name);
		static void BindSampler(NVRHI::DrawCallState& state, const String& name, int slot);
	};
}