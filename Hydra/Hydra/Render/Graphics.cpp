#include "Hydra/Render/Graphics.h"
#include "Hydra/EngineContext.h"

#include "Hydra/Render/Pipeline/BindingHelpers.h"

#include "Hydra/Render/Shader.h"

FGraphics::~FGraphics()
{
	delete _BlitMaterial;

	ITER(_ConstantBuffers, it)
	{
		_Context->GetRenderInterface()->destroyConstantBuffer(it->second.Handle);
	}

	ITER(_RenderViewTargets, it)
	{
		_Context->GetRenderInterface()->destroyTexture(it->second);
	}

	ITER(_InputLayouts, it)
	{
		_Context->GetRenderInterface()->destroyInputLayout(it->second);
	}
}

FGraphics::FGraphics(EngineContext* context) : _Context(context)
{
	_BlitMaterial = new MaterialInterface("Blit", MakeShared<Technique>(context, "Assets/Shaders/Blit.hlsl", true));
	//_BlitMaterial = Material::CreateOrGet("Assets/Shaders/Blit.hlsl", true, true);
	//_BlurMaterial = Material::CreateOrGet("Assets/Shaders/PostProcess/GaussianBlur.hlsl", true, true);
}

void FGraphics::AllocateViewDependentResources(uint32 width, uint32 height, uint32 sampleCount)
{
	ReleaseRenderTarget("G_MEM_BLUR_PASS");
	CreateRenderTarget("G_MEM_BLUR_PASS", NVRHI::Format::RGBA8_UNORM, width, height, NVRHI::Color(1.f), sampleCount);
}

void FGraphics::Blit(TexturePtr pSource, TexturePtr pDest)
{
	NVRHI::DrawCallState state;

	state.primType = NVRHI::PrimitiveType::TRIANGLE_STRIP;
	SetMaterialShaders(state, _BlitMaterial);

	state.renderState.targetCount = 1;
	state.renderState.targets[0] = pDest;
	state.renderState.viewportCount = 1;
	state.renderState.viewports[0] = NVRHI::Viewport(float(_Context->ScreenSize.x), float(_Context->ScreenSize.y));
	state.renderState.depthStencilState.depthEnable = false;
	state.renderState.rasterState.cullMode = NVRHI::RasterState::CULL_NONE;

	_BlitMaterial->SetTexture("_Texture", pSource);

	ApplyMaterialParameters(state, _BlitMaterial);

	NVRHI::DrawArguments args;
	args.vertexCount = 4;
	_Context->GetRenderInterface()->draw(state, &args, 1);
}

void FGraphics::Blit(const String & name, TexturePtr pDest)
{
	Blit(GetRenderTarget(name), pDest);
}

void FGraphics::Blit(const String & pSource, const String & pDest)
{
	Blit(GetRenderTarget(pSource), GetRenderTarget(pDest));
}

void FGraphics::BlurTexture(TexturePtr pSource, TexturePtr pDest)
{
	//TODO: Different size of textures

	NVRHI::TextureDesc desc = pDest->GetDesc();

	float width = (float)_Context->ScreenSize.x;
	float height = (float)_Context->ScreenSize.y;

	// Horizontal blur
	Composite(_BlurMaterial, [this, pSource, width, height](NVRHI::DrawCallState& state)
	{
		_BlurMaterial->SetTexture("_Texture", pSource);

		_BlurMaterial->SetVector2("_Direction", Vector2(1, 0));
		_BlurMaterial->SetVector2("_TexSize", Vector2(width, height));

		ApplyMaterialParameters(state, _BlurMaterial);

	}, "G_MEM_BLUR_PASS");

	// Vertical blur
	Composite(_BlurMaterial, [this, pSource, width, height](NVRHI::DrawCallState& state)
	{
		_BlurMaterial->SetTexture("_Texture", GetRenderTarget("G_MEM_BLUR_PASS"));

		_BlurMaterial->SetVector2("_Direction", Vector2(0, 1));
		_BlurMaterial->SetVector2("_TexSize", Vector2(width, height));

		ApplyMaterialParameters(state, _BlurMaterial);

	}, pDest);
}

void FGraphics::BlurTexture(const String pSource, const String pDest)
{
	BlurTexture(GetRenderTarget(pSource), GetRenderTarget(pDest));
}

void FGraphics::Composite(MaterialInterface* material, Function<void(NVRHI::DrawCallState&)> preRenderFunction, TexturePtr pDest)
{
	NVRHI::DrawCallState state;

	state.primType = NVRHI::PrimitiveType::TRIANGLE_STRIP;

	state.renderState.targetCount = 1;
	state.renderState.targets[0] = pDest;
	state.renderState.viewportCount = 1;
	state.renderState.viewports[0] = NVRHI::Viewport(float(_Context->ScreenSize.x), float(_Context->ScreenSize.y));
	state.renderState.depthStencilState.depthEnable = false;
	state.renderState.rasterState.cullMode = NVRHI::RasterState::CULL_NONE;

	SetMaterialShaders(state, material);

	if (preRenderFunction)
		preRenderFunction(state);

	material->ApplyParams(state);

	NVRHI::DrawArguments args;
	args.vertexCount = 4;
	_Context->GetRenderInterface()->draw(state, &args, 1);
}

void FGraphics::Composite(MaterialInterface* material, Function<void(NVRHI::DrawCallState&)> preRenderFunction, const String& outputName)
{
	Composite(material, preRenderFunction, GetRenderTarget(outputName));
}

void FGraphics::Composite(MaterialInterface* material, TexturePtr slot0Texture, TexturePtr pDest)
{
	Composite(material, [material, slot0Texture](NVRHI::DrawCallState& state)
	{
		material->SetTexture("_Texture", slot0Texture);
	}, pDest);
}

void FGraphics::Composite(MaterialInterface* material, const String & slot0Texture, const String & pDest)
{
	Composite(material, GetRenderTarget(slot0Texture), GetRenderTarget(pDest));
}

void FGraphics::Dispatch(MaterialInterface* material, uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ)
{
	NVRHI::DispatchState state;

	state.shader = material->GetRawShader(NVRHI::ShaderType::SHADER_COMPUTE);

	if (state.shader == nullptr)
	{
		throw;
		LogError("Graphics::Composite", material->Name + ", " + ToString(groupsX) + ", " + ToString(groupsY) + ", " + ToString(groupsZ), "Material does not have compute shader !");
		return;
	}

	material->ApplyParams(state);

	_Context->GetRenderInterface()->dispatch(state, groupsX, groupsY, groupsZ);
}

void FGraphics::RenderCubeMap(MaterialInterface* material, InputLayoutPtr inputLayout, const Vector2& viewPort, Function<void(NVRHI::DrawCallState&, int, int)> preRenderFunction, TexturePtr pDest)
{
	_Context->GetRenderInterface()->beginRenderingPass();

	NVRHI::DrawCallState state;
	FGraphics::SetClearFlags(state, MakeRGBf(0.2f, 0.2f, 0.2f));

	state.renderState.viewportCount = 1;
	state.renderState.viewports[0] = NVRHI::Viewport(viewPort.x, viewPort.y);

	state.renderState.targetCount = 1;
	state.renderState.targets[0] = pDest;

	state.inputLayout = inputLayout;
	SetMaterialShaders(state, material);

	state.renderState.rasterState.cullMode = NVRHI::RasterState::CULL_NONE;
	state.renderState.depthStencilState.depthEnable = true;

	for (uint32_t mipLevel = 0; mipLevel < pDest->GetDesc().mipLevels; mipLevel++)
	{
		state.renderState.clearColorTarget = true;
		state.renderState.clearDepthTarget = true;

		for (int i = 0; i < 6; i++)
		{
			state.renderState.targetMipSlices[0] = mipLevel;
			state.renderState.targetIndicies[0] = i;

			if (preRenderFunction)
				preRenderFunction(state, mipLevel, i);

			state.renderState.clearColorTarget = false;
			state.renderState.clearDepthTarget = false;
		}
	}

	_Context->GetRenderInterface()->endRenderingPass();
}

void FGraphics::RenderCubeMap(MaterialInterface* material, const String& inputLayout, const Vector2& viewPort, Function<void(NVRHI::DrawCallState&, int, int)> preRenderFunction, const String & outputName)
{
	RenderCubeMap(material, GetInputLayout(inputLayout), viewPort, preRenderFunction, GetRenderTarget(outputName));
}

void FGraphics::SetMaterialShaders(NVRHI::DrawCallState& state, MaterialInterface* material)
{
	state.VS.shader = material->GetRawShader(NVRHI::ShaderType::SHADER_VERTEX);
	state.HS.shader = material->GetRawShader(NVRHI::ShaderType::SHADER_HULL);
	state.DS.shader = material->GetRawShader(NVRHI::ShaderType::SHADER_DOMAIN);
	state.GS.shader = material->GetRawShader(NVRHI::ShaderType::SHADER_GEOMETRY);
	state.PS.shader = material->GetRawShader(NVRHI::ShaderType::SHADER_PIXEL);
}

void FGraphics::ApplyMaterialParameters(NVRHI::DrawCallState & state, MaterialInterface* mateiral)
{
	mateiral->ApplyParams(state);
}

void FGraphics::SetClearFlags(NVRHI::DrawCallState& state, const ColorRGBA& color)
{
	state.renderState.clearColor = NVRHI::Color(color.r, color.g, color.b, color.a);
	state.renderState.clearColorTarget = true;
	state.renderState.clearDepthTarget = true;
}

NVRHI::ConstantBufferHandle FGraphics::CreateConstantBuffer(uint32_t size, const String & mappedName, const unsigned int specificBinding, int slot)
{
	if (_ConstantBuffers.find(mappedName) != _ConstantBuffers.end())
	{
		LogError("Graphics::CreateConstantBuffer", ToString(size) + ", " + mappedName + ", " + ToString(specificBinding), "Name is already exist!");
		return nullptr;
	}

	ConstantBufferInfo info = {};
	info.Name = mappedName;
	info.DataSize = size;
	info.Slot = slot;
	info.SpecificBinding = specificBinding;
	info.Handle = _Context->GetRenderInterface()->createConstantBuffer(NVRHI::ConstantBufferDesc(size, nullptr), nullptr);

	_ConstantBuffers[mappedName] = info;

	return info.Handle;
}

void FGraphics::WriteConstantBufferData(NVRHI::ConstantBufferHandle handle, const void* data, uint32_t size)
{
	_Context->GetRenderInterface()->writeConstantBuffer(handle, data, size);
}

void FGraphics::WriteConstantBufferData(const String& mappedName, const void* data)
{
	if (_ConstantBuffers.find(mappedName) == _ConstantBuffers.end())
	{
		LogError("Graphics::WriteConstantBufferData", mappedName + ", RAW_DATA", "Constant buffer not found !");
		return;
	}

	ConstantBufferInfo& info = _ConstantBuffers[mappedName];

	_Context->GetRenderInterface()->writeConstantBuffer(info.Handle, data, info.DataSize);
}

void FGraphics::WriteConstantBufferDataAndBind(NVRHI::DrawCallState& state, const String& mappedName, const void* data)
{
	WriteConstantBufferData(mappedName, data);
	BindConstantBuffer(state, mappedName);
}

void FGraphics::BindConstantBuffer(NVRHI::PipelineStageBindings& ds, uint32_t slot, NVRHI::ConstantBufferHandle handle)
{
	NVRHI::BindConstantBuffer(ds, slot, handle);
}

void FGraphics::BindConstantBuffer(NVRHI::DrawCallState & state, const String & mappedName, uint32_t slot, bool slotOverride)
{
	if (_ConstantBuffers.find(mappedName) == _ConstantBuffers.end())
	{
		LogError("Graphics::BindConstantBuffer", "DrawCallState" + mappedName + ", " + ToString(slot), "Constant buffer not found !");
		return;
	}

	ConstantBufferInfo& info = _ConstantBuffers[mappedName];

	uint32_t selectedSlot = info.Slot;

	if (selectedSlot < 0 || slotOverride)
	{
		selectedSlot = slot;
	}

	NVRHI::PipelineStageBindings* binding = nullptr;

	if (info.SpecificBinding & PSB_VERTEX)
	{
		binding = &state.VS;
	}
	else if (info.SpecificBinding & PSB_HULL)
	{
		binding = &state.HS;
	}
	else if (info.SpecificBinding & PSB_DOMAIN)
	{
		binding = &state.DS;
	}
	else if (info.SpecificBinding & PSB_GEOMETRY)
	{
		binding = &state.GS;
	}
	else if (info.SpecificBinding & PSB_PIXEL)
	{
		binding = &state.PS;
	}
	else if (info.SpecificBinding & PSB_COMPUTE)
	{
		LogError("Graphics::BindConstantBuffer", "DrawCallState" + mappedName + ", " + ToString(slot), "Compute binding is not supported !");
		return;
	}

	if (binding != nullptr)
	{
		BindConstantBuffer(*binding, selectedSlot, info.Handle);
	}
}

ConstantBufferPtr FGraphics::GetConstantBuffer(const String & mappedName)
{
	if (_ConstantBuffers.find(mappedName) == _ConstantBuffers.end())
	{
		//LogError("Graphics::BindConstantBuffer", "DrawCallState" + mappedName + ", " + ToString(slot), "Constant buffer not found !");
		return nullptr;
	}

	ConstantBufferInfo& info = _ConstantBuffers[mappedName];

	return info.Handle;
}

TexturePtr FGraphics::CreateRenderTarget(const String & name, const NVRHI::Format::Enum & format, UINT width, UINT height, const NVRHI::Color & clearColor, UINT sampleCount)
{
	if (_RenderViewTargets.find(name) != _RenderViewTargets.end())
	{
		return _RenderViewTargets[name];
	}

	NVRHI::TextureDesc gbufferDesc;
	gbufferDesc.width = width;
	gbufferDesc.height = height;
	gbufferDesc.isRenderTarget = true;
	gbufferDesc.useClearValue = true;
	gbufferDesc.sampleCount = sampleCount;
	gbufferDesc.disableGPUsSync = true;

	gbufferDesc.format = format;
	gbufferDesc.clearValue = clearColor;
	gbufferDesc.debugName = name;
	NVRHI::TextureHandle handle = _Context->GetRenderInterface()->createTexture(gbufferDesc, NULL);
	_RenderViewTargets[name] = handle;
	return handle;
}

TexturePtr FGraphics::CreateRenderTarget(const String & name, const NVRHI::Format::Enum & format, const Vector2i & size)
{
	return CreateRenderTarget(name, format, size.x, size.y, NVRHI::Color(0, 0, 0, 1), 1);
}

TexturePtr FGraphics::CreateRenderTarget2DArray(const String & name, const NVRHI::Format::Enum & format, UINT width, UINT height, int mipCount, int arrSize)
{
	if (_RenderViewTargets.find(name) != _RenderViewTargets.end())
	{
		return _RenderViewTargets[name];
	}

	NVRHI::TextureDesc gbufferDesc;
	gbufferDesc.width = width;
	gbufferDesc.height = width;
	gbufferDesc.isRenderTarget = true;
	gbufferDesc.useClearValue = false;
	gbufferDesc.sampleCount = 1;
	gbufferDesc.sampleQuality = 0;
	gbufferDesc.disableGPUsSync = true;

	gbufferDesc.mipLevels = mipCount;
	gbufferDesc.depthOrArraySize = arrSize;
	gbufferDesc.isArray = true;

	gbufferDesc.format = format;
	gbufferDesc.debugName = name;

	NVRHI::TextureHandle handle = _Context->GetRenderInterface()->createTexture(gbufferDesc, NULL);

	_RenderViewTargets[name] = handle;

	return handle;
}

TexturePtr FGraphics::CreateRenderTargetCubeMap(const String & name, const NVRHI::Format::Enum & format, UINT width, UINT height, const NVRHI::Color& clearColor, int mipLevels)
{
	if (_RenderViewTargets.find(name) != _RenderViewTargets.end())
	{
		return _RenderViewTargets[name];
	}

	NVRHI::TextureDesc gbufferDesc;
	gbufferDesc.width = width;
	gbufferDesc.height = width;
	gbufferDesc.isRenderTarget = true;
	gbufferDesc.useClearValue = true;
	gbufferDesc.clearValue = clearColor;
	gbufferDesc.sampleCount = 1;
	gbufferDesc.sampleQuality = 0;
	gbufferDesc.disableGPUsSync = true;

	gbufferDesc.mipLevels = mipLevels;
	gbufferDesc.depthOrArraySize = 6;
	gbufferDesc.isArray = false;
	gbufferDesc.isCubeMap = true;

	gbufferDesc.format = format;
	gbufferDesc.debugName = name;

	NVRHI::TextureHandle handle = _Context->GetRenderInterface()->createTexture(gbufferDesc, NULL);

	_RenderViewTargets[name] = handle;

	return handle;
}

TexturePtr FGraphics::CreateUAVTexture(const String& name, const NVRHI::Format::Enum & format, UINT width, UINT height, const NVRHI::Color & clearColor, int mipLevels)
{
	if (_RenderViewTargets.find(name) != _RenderViewTargets.end())
	{
		return _RenderViewTargets[name];
	}

	NVRHI::TextureDesc gbufferDesc;
	gbufferDesc.width = width;
	gbufferDesc.height = height;
	//gbufferDesc.useClearValue = true;
	//gbufferDesc.sampleCount = 1;
	//gbufferDesc.disableGPUsSync = false;
	gbufferDesc.isRenderTarget = true;
	gbufferDesc.isUAV = true;
	//gbufferDesc.usage = NVRHI::TextureDesc::Usage::USAGE_DYNAMIC;

	gbufferDesc.format = format;
	gbufferDesc.clearValue = clearColor;
	gbufferDesc.debugName = name;
	NVRHI::TextureHandle handle = _Context->GetRenderInterface()->createTexture(gbufferDesc, NULL);
	_RenderViewTargets[name] = handle;
	return handle;
}

TexturePtr FGraphics::CreateUAVTexture3D(const String & name, const NVRHI::Format::Enum & format, UINT width, UINT height, UINT depth, const NVRHI::Color & clearColor, int mipLevels)
{
	if (_RenderViewTargets.find(name) != _RenderViewTargets.end())
	{
		return _RenderViewTargets[name];
	}

	NVRHI::TextureDesc gbufferDesc;
	gbufferDesc.width = width;
	gbufferDesc.height = height;
	//gbufferDesc.useClearValue = true;
	//gbufferDesc.sampleCount = 1;
	//gbufferDesc.disableGPUsSync = false;
	gbufferDesc.isRenderTarget = true;
	gbufferDesc.isUAV = true;
	gbufferDesc.depthOrArraySize = depth;
	//gbufferDesc.usage = NVRHI::TextureDesc::Usage::USAGE_DYNAMIC;

	gbufferDesc.format = format;
	gbufferDesc.clearValue = clearColor;
	gbufferDesc.debugName = name;
	NVRHI::TextureHandle handle = _Context->GetRenderInterface()->createTexture(gbufferDesc, NULL);
	_RenderViewTargets[name] = handle;
	return handle;
}

TexturePtr FGraphics::ResizeRenderTarget(TexturePtr texture, int w, int h)
{
	if (texture)
	{
		return ResizeRenderTarget(texture->GetDesc().debugName, w, h);
	}

	return nullptr;
}

TexturePtr FGraphics::ResizeRenderTarget(const String& textureName, int w, int h)
{
	TexturePtr texture = GetRenderTarget(textureName);

	if (texture)
	{
		NVRHI::TextureDesc desc = texture->GetDesc();

		ReleaseRenderTarget(texture);

		return CreateRenderTarget(desc.debugName, desc.format, w, h, desc.clearValue, desc.sampleCount);
	}

	return nullptr;
}

TexturePtr FGraphics::GetRenderTarget(const String & name)
{
	if (_RenderViewTargets.find(name) != _RenderViewTargets.end())
	{
		return _RenderViewTargets[name];
	}
	return nullptr;
}

void FGraphics::ReleaseRenderTarget(const String & name)
{
	if (_RenderViewTargets.find(name) != _RenderViewTargets.end())
	{
		TexturePtr rt = _RenderViewTargets[name];

		_RenderViewTargets.erase(name);

		_Context->GetRenderInterface()->destroyTexture(rt);
	}
}

void FGraphics::ReleaseRenderTarget(TexturePtr texture)
{
	if (texture)
	{
		ReleaseRenderTarget(texture->GetDesc().debugName);
	}
}

void FGraphics::BindRenderTarget(NVRHI::DrawCallState & state, const String & name, int index)
{
	if (_RenderViewTargets.find(name) != _RenderViewTargets.end())
	{
		TexturePtr rt = _RenderViewTargets[name];

		NVRHI::BindTexture(state.PS, index, rt, false, rt->GetDesc().format, rt->GetDesc().mipLevels);
	}
}

InputLayoutPtr FGraphics::CreateInputLayout(const String& name, const NVRHI::VertexAttributeDesc * d, uint32_t attributeCount, MaterialInterface* material)
{
	if (_InputLayouts.find(name) != _InputLayouts.end())
	{
		return _InputLayouts[name];
	}

	ID3DBlob* blob = material->GetShader(NVRHI::ShaderType::SHADER_VERTEX)->GetBlob();

	InputLayoutPtr layout = _Context->GetRenderInterface()->createInputLayout(d, attributeCount, blob->GetBufferPointer(), blob->GetBufferSize());

	_InputLayouts[name] = layout;

	return layout;
}

InputLayoutPtr FGraphics::GetInputLayout(const String & name)
{
	if (_InputLayouts.find(name) != _InputLayouts.end())
	{
		return _InputLayouts[name];
	}

	return nullptr;
}

SamplerPtr FGraphics::CreateSampler(const String & name, const WrapMode & wrapX, const WrapMode & wrapY, const WrapMode & wrapZ, bool minFilter, bool magFilter, bool mipFilter, int anisotropy)
{
	if (_Samplers.find(name) != _Samplers.end())
	{
		return _Samplers[name];
	}

	NVRHI::SamplerDesc samplerDesc;
	samplerDesc.wrapMode[0] = wrapX;
	samplerDesc.wrapMode[1] = wrapY;
	samplerDesc.wrapMode[2] = wrapZ;
	samplerDesc.minFilter = minFilter;
	samplerDesc.magFilter = magFilter;
	samplerDesc.mipFilter = mipFilter;
	samplerDesc.anisotropy = 16;

	SamplerPtr sampler = _Context->GetRenderInterface()->createSampler(samplerDesc);

	_Samplers[name] = sampler;

	return sampler;
}

SamplerPtr FGraphics::CreateShadowCompareSampler(const String & name)
{
	if (_Samplers.find(name) != _Samplers.end())
	{
		return _Samplers[name];
	}

	NVRHI::SamplerDesc samplerComparisonDesc;
	samplerComparisonDesc.wrapMode[0] = NVRHI::SamplerDesc::WRAP_MODE_BORDER;
	samplerComparisonDesc.wrapMode[1] = NVRHI::SamplerDesc::WRAP_MODE_BORDER;
	samplerComparisonDesc.wrapMode[2] = NVRHI::SamplerDesc::WRAP_MODE_BORDER;
	samplerComparisonDesc.minFilter = samplerComparisonDesc.magFilter = true;
	samplerComparisonDesc.shadowCompare = true;
	samplerComparisonDesc.borderColor = NVRHI::Color(0.f);

	SamplerPtr sampler = _Context->GetRenderInterface()->createSampler(samplerComparisonDesc);

	_Samplers[name] = sampler;

	return sampler;
}

SamplerPtr FGraphics::GetSampler(const String & name)
{
	if (_Samplers.find(name) != _Samplers.end())
	{
		return _Samplers[name];
	}

	return nullptr;
}
void FGraphics::BindSampler(NVRHI::DrawCallState & state, const String & name, int slot)
{
	if (_Samplers.find(name) != _Samplers.end())
	{
		SamplerPtr sampler = _Samplers[name];

		NVRHI::BindSampler(state.PS, slot, sampler);
	}
}