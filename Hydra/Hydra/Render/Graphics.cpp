#include "Hydra/Render/Graphics.h"
#include "Hydra/Engine.h"

#include "Hydra/Render/Pipeline/BindingHelpers.h"

#include "Hydra/Import/ShaderImporter.h"

namespace Hydra
{
	Map<String, ConstantBufferInfo> Graphics::_ConstantBuffers;
	Map<String, NVRHI::TextureHandle> Graphics::_RenderViewTargets;
	Map<String, InputLayoutPtr> Graphics::_InputLayouts;
	Map<String, SamplerPtr> Graphics::_Samplers;
	ShaderPtr Graphics::_BlitShader;

	void Graphics::Destroy()
	{
		ITER(_ConstantBuffers, it)
		{
			Engine::GetRenderInterface()->destroyConstantBuffer(it->second.Handle);
		}

		ITER(_RenderViewTargets, it)
		{
			Engine::GetRenderInterface()->destroyTexture(it->second);
		}

		ITER(_InputLayouts, it)
		{
			Engine::GetRenderInterface()->destroyInputLayout(it->second);
		}
	}

	void Graphics::Create()
	{
		_BlitShader = ShaderImporter::Import("Assets/Shaders/blit.hlsl");
	}

	void Graphics::Blit(TexturePtr pSource, TexturePtr pDest)
	{
		NVRHI::DrawCallState state;

		state.primType = NVRHI::PrimitiveType::TRIANGLE_STRIP;
		state.VS.shader = _BlitShader->GetShader(NVRHI::ShaderType::SHADER_VERTEX);
		state.PS.shader = _BlitShader->GetShader(NVRHI::ShaderType::SHADER_PIXEL);

		state.renderState.targetCount = 1;
		state.renderState.targets[0] = pDest;
		state.renderState.viewportCount = 1;
		state.renderState.viewports[0] = NVRHI::Viewport(float(Engine::ScreenSize.x), float(Engine::ScreenSize.y));
		state.renderState.depthStencilState.depthEnable = false;
		state.renderState.rasterState.cullMode = NVRHI::RasterState::CULL_NONE;

		NVRHI::BindTexture(state.PS, 0, pSource);

		NVRHI::DrawArguments args;
		args.vertexCount = 4;
		Engine::GetRenderInterface()->draw(state, &args, 1);
	}

	void Graphics::Blit(const String & name, TexturePtr pDest)
	{
		Blit(GetRenderTarget(name), pDest);
	}

	void Graphics::Composite(ShaderPtr shader, Function<void(NVRHI::DrawCallState&)> preRenderFunction, TexturePtr pDest)
	{
		NVRHI::DrawCallState state;

		state.primType = NVRHI::PrimitiveType::TRIANGLE_STRIP;
		state.VS.shader = shader->GetShader(NVRHI::ShaderType::SHADER_VERTEX);
		state.PS.shader = shader->GetShader(NVRHI::ShaderType::SHADER_PIXEL);

		state.renderState.targetCount = 1;
		state.renderState.targets[0] = pDest;
		state.renderState.viewportCount = 1;
		state.renderState.viewports[0] = NVRHI::Viewport(float(Engine::ScreenSize.x), float(Engine::ScreenSize.y));
		state.renderState.depthStencilState.depthEnable = false;
		state.renderState.rasterState.cullMode = NVRHI::RasterState::CULL_NONE;

		if(preRenderFunction)
			preRenderFunction(state);

		NVRHI::DrawArguments args;
		args.vertexCount = 4;
		Engine::GetRenderInterface()->draw(state, &args, 1);
	}

	void Graphics::Composite(ShaderPtr shader, Function<void(NVRHI::DrawCallState&)> preRenderFunction, const String& outputName)
	{
		Composite(shader, preRenderFunction, GetRenderTarget(outputName));
	}

	void Graphics::SetShader(NVRHI::DrawCallState& state, ShaderPtr shader)
	{
		state.VS.shader = shader->GetShader(NVRHI::ShaderType::SHADER_VERTEX);
		state.HS.shader = shader->GetShader(NVRHI::ShaderType::SHADER_HULL);
		state.DS.shader = shader->GetShader(NVRHI::ShaderType::SHADER_DOMAIN);
		state.GS.shader = shader->GetShader(NVRHI::ShaderType::SHADER_GEOMETRY);
		state.PS.shader = shader->GetShader(NVRHI::ShaderType::SHADER_PIXEL);
	}

	void Graphics::SetClearFlags(NVRHI::DrawCallState& state, const ColorRGBA& color)
	{
		state.renderState.clearColor = NVRHI::Color(color.r, color.g, color.b, color.a);
		state.renderState.clearColorTarget = true;
		state.renderState.clearDepthTarget = true;
	}

	NVRHI::ConstantBufferHandle Graphics::CreateConstantBuffer(uint32_t size, const String & mappedName, const unsigned int specificBinding, int slot)
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
		info.Handle = Engine::GetRenderInterface()->createConstantBuffer(NVRHI::ConstantBufferDesc(size, nullptr), nullptr);

		_ConstantBuffers[mappedName] = info;

		return info.Handle;
	}

	void Graphics::WriteConstantBufferData(NVRHI::ConstantBufferHandle handle, const void* data, uint32_t size)
	{
		Engine::GetRenderInterface()->writeConstantBuffer(handle, data, size);
	}

	void Graphics::WriteConstantBufferData(const String& mappedName, const void* data)
	{
		if (_ConstantBuffers.find(mappedName) == _ConstantBuffers.end())
		{
			LogError("Graphics::WriteConstantBufferData", mappedName + ", RAW_DATA", "Constant buffer not found !");
			return;
		}

		ConstantBufferInfo& info = _ConstantBuffers[mappedName];

		Engine::GetRenderInterface()->writeConstantBuffer(info.Handle, data, info.DataSize);
	}

	void Graphics::WriteConstantBufferDataAndBind(NVRHI::DrawCallState& state, const String& mappedName, const void* data)
	{
		WriteConstantBufferData(mappedName, data);
		BindConstantBuffer(state, mappedName);
	}

	void Graphics::BindConstantBuffer(NVRHI::PipelineStageBindings& ds, uint32_t slot, NVRHI::ConstantBufferHandle handle)
	{
		NVRHI::BindConstantBuffer(ds, slot, handle);
	}

	void Graphics::BindConstantBuffer(NVRHI::DrawCallState & state, const String & mappedName, uint32_t slot, bool slotOverride)
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
		} else if (info.SpecificBinding & PSB_HULL)
		{
			binding = &state.HS;
		} else if (info.SpecificBinding & PSB_DOMAIN)
		{
			binding = &state.DS;
		} else if (info.SpecificBinding & PSB_GEOMETRY)
		{
			binding = &state.GS;
		} else if (info.SpecificBinding & PSB_PIXEL)
		{
			binding = &state.PS;
		} else if (info.SpecificBinding & PSB_COMPUTE)
		{
			LogError("Graphics::BindConstantBuffer", "DrawCallState" + mappedName + ", " + ToString(slot), "Compute binding is not supported !");
			return;
		}

		if (binding != nullptr)
		{
			BindConstantBuffer(*binding, selectedSlot, info.Handle);
		}
	}

	TexturePtr Graphics::CreateRenderTarget(const String & name, const NVRHI::Format::Enum & format, UINT width, UINT height, const NVRHI::Color & clearColor, UINT sampleCount)
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
		gbufferDesc.debugName = name.c_str();
		NVRHI::TextureHandle handle = Engine::GetRenderInterface()->createTexture(gbufferDesc, NULL);
		_RenderViewTargets[name] = handle;
		return handle;
	}

	TexturePtr Graphics::GetRenderTarget(const String & name)
	{
		if (_RenderViewTargets.find(name) != _RenderViewTargets.end())
		{
			return _RenderViewTargets[name];
		}
		return nullptr;
	}

	void Graphics::ReleaseRenderTarget(const String & name)
	{
		if (_RenderViewTargets.find(name) != _RenderViewTargets.end())
		{
			TexturePtr rt = _RenderViewTargets[name];

			Engine::GetRenderInterface()->destroyTexture(rt);

			_RenderViewTargets.erase(name);
		}
	}

	void Graphics::BindRenderTarget(NVRHI::DrawCallState & state, const String & name, int index)
	{
		if (_RenderViewTargets.find(name) != _RenderViewTargets.end())
		{
			TexturePtr rt = _RenderViewTargets[name];

			NVRHI::BindTexture(state.PS, index, rt);
		}
	}

	InputLayoutPtr Graphics::CreateInputLayout(const String & name, const NVRHI::VertexAttributeDesc * d, uint32_t attributeCount, ShaderPtr shader)
	{
		if (_InputLayouts.find(name) != _InputLayouts.end())
		{
			return _InputLayouts[name];
		}

		ID3DBlob* blob = shader->GetShaderBlob(NVRHI::ShaderType::SHADER_VERTEX);
		InputLayoutPtr layout = Engine::GetRenderInterface()->createInputLayout(d, attributeCount, blob->GetBufferPointer(), blob->GetBufferSize());

		_InputLayouts[name] = layout;

		return layout;
	}

	InputLayoutPtr Graphics::GetInputLayout(const String & name)
	{
		if (_InputLayouts.find(name) != _InputLayouts.end())
		{
			return _InputLayouts[name];
		}

		return nullptr;
	}

	SamplerPtr Graphics::CreateSampler(const String & name, const WrapMode & wrapX, const WrapMode & wrapY, const WrapMode & wrapZ, bool minFilter, bool magFilter, bool mipFilter, int anisotropy)
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

		SamplerPtr sampler = Engine::GetRenderInterface()->createSampler(samplerDesc);

		_Samplers[name] = sampler;

		return sampler;
	}

	SamplerPtr Graphics::GetSampler(const String & name)
	{
		if (_Samplers.find(name) != _Samplers.end())
		{
			return _Samplers[name];
		}

		return nullptr;
	}
}