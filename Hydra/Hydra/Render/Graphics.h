#pragma once

#include "Hydra/Core/ColorRGBA.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/String.h"
#include "Hydra/Core/Function.h"

#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"
#include "Hydra/Render/Material.h"

enum PipelineStageBindingType : unsigned int
{
	PSB_VERTEX = 1 << 1,
	PSB_HULL = 1 << 2,
	PSB_DOMAIN = 1 << 3,
	PSB_GEOMETRY = 1 << 4,
	PSB_PIXEL = 1 << 5,
	PSB_SHADERS_NUM = 1 << 6,
	PSB_COMPUTE = 1 << 7
};

typedef NVRHI::InputLayoutHandle InputLayoutPtr;
typedef NVRHI::TextureHandle TexturePtr;
typedef NVRHI::ConstantBufferHandle ConstantBufferPtr;
typedef NVRHI::SamplerHandle SamplerPtr;

typedef NVRHI::SamplerDesc::WrapMode WrapMode;

#define CmpFnc Function<void(NVRHI::DrawCallState&, int, int)>

class HYDRA_API EngineContext;

struct ConstantBufferInfo
{
	String Name;
	uint32_t DataSize;
	unsigned int SpecificBinding;
	int Slot;
	ConstantBufferPtr Handle;
};

class HYDRA_API Graphics
{
private:
	EngineContext* _Context;

	Map<String, ConstantBufferInfo> _ConstantBuffers;
	Map<String, TexturePtr> _RenderViewTargets;
	Map<String, InputLayoutPtr> _InputLayouts;
	Map<String, SamplerPtr> _Samplers;

	MaterialInterface* _BlitMaterial;
	MaterialInterface* _BlurMaterial;
public:
	Graphics(EngineContext* context);
	~Graphics();

	void AllocateViewDependentResources(uint32 width, uint32 height, uint32 sampleCount);

	void Blit(TexturePtr pSource, TexturePtr pDest);
	void Blit(const String& name, TexturePtr pDest);
	void Blit(const String& pSource, const String& pDest);

	void BlurTexture(TexturePtr pSource, TexturePtr pDest);
	void BlurTexture(const String pSource, const String pDest);

	void Composite(MaterialInterface* mateiral, Function<void(NVRHI::DrawCallState&)> preRenderFunction, TexturePtr pDest);
	void Composite(MaterialInterface* mateiral, Function<void(NVRHI::DrawCallState&)> preRenderFunction, const String& outputName);
	void Composite(MaterialInterface* mateiral, TexturePtr slot0Texture, TexturePtr pDest);
	void Composite(MaterialInterface* mateiral, const String& slot0Texture, const String& pDest);

	void Dispatch(MaterialInterface* material, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ);

	void RenderCubeMap(MaterialInterface* mateiral, InputLayoutPtr inputLayout, const Vector2& viewPort, Function<void(NVRHI::DrawCallState&, int, int)> preRenderFunction, TexturePtr pDest);
	void RenderCubeMap(MaterialInterface* mateiral, const String& inputLayout, const Vector2& viewPort, Function<void(NVRHI::DrawCallState&, int, int)> preRenderFunction, const String& outputName);

	void SetMaterialShaders(NVRHI::DrawCallState& state, MaterialInterface* mateiral);
	void ApplyMaterialParameters(NVRHI::DrawCallState& state, MaterialInterface* mateiral);

	void SetClearFlags(NVRHI::DrawCallState& state, const ColorRGBA& color);

	NVRHI::ConstantBufferHandle CreateConstantBuffer(uint32_t size, const String& mappedName = String_None, const unsigned int specificBinding = 0, int slot = -1);

	void WriteConstantBufferData(NVRHI::ConstantBufferHandle handle, const void* data, uint32_t size);
	void WriteConstantBufferData(const String& mappedName, const void* data);
	void WriteConstantBufferDataAndBind(NVRHI::DrawCallState& state, const String& mappedName, const void* data);

	void BindConstantBuffer(NVRHI::PipelineStageBindings& ds, uint32_t slot, NVRHI::ConstantBufferHandle handle);
	void BindConstantBuffer(NVRHI::DrawCallState& state, const String& mappedName, uint32_t slot = 0, bool slotOverride = false);

	ConstantBufferPtr GetConstantBuffer(const String& mappedName);

	TexturePtr CreateRenderTarget(const String& name, const NVRHI::Format::Enum& format, UINT width, UINT height, const NVRHI::Color& clearColor, UINT sampleCount);
	TexturePtr CreateRenderTarget2DArray(const String& name, const NVRHI::Format::Enum& format, UINT width, UINT height, int mipCount, int arrSize);
	TexturePtr CreateRenderTargetCubeMap(const String& name, const NVRHI::Format::Enum& format, UINT width, UINT height, const NVRHI::Color& clearColor, int mipLevels = 1);
	TexturePtr CreateUAVTexture(const String& name, const NVRHI::Format::Enum& format, UINT width, UINT height, const NVRHI::Color& clearColor = NVRHI::Color(0.0f), int mipLevels = 1);
	TexturePtr CreateUAVTexture3D(const String& name, const NVRHI::Format::Enum& format, UINT width, UINT height, UINT depth, const NVRHI::Color& clearColor = NVRHI::Color(0.0f), int mipLevels = 1);

	TexturePtr GetRenderTarget(const String& name);
	void ReleaseRenderTarget(const String& name);
	void BindRenderTarget(NVRHI::DrawCallState& state, const String& name, int index);

	InputLayoutPtr CreateInputLayout(const String& name, const NVRHI::VertexAttributeDesc* d, uint32_t attributeCount, MaterialInterface* material);
	InputLayoutPtr GetInputLayout(const String& name);

	SamplerPtr CreateSampler(const String& name, const WrapMode& wrapX = WrapMode::WRAP_MODE_WRAP, const WrapMode& wrapY = WrapMode::WRAP_MODE_WRAP, const WrapMode& wrapZ = WrapMode::WRAP_MODE_WRAP, bool minFilter = true, bool magFilter = true, bool mipFilter = true, int anisotropy = 16);
	SamplerPtr CreateShadowCompareSampler(const String& name);
	SamplerPtr GetSampler(const String& name);
	void BindSampler(NVRHI::DrawCallState& state, const String& name, int slot);
};