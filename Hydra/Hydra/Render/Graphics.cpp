#include "Hydra/Render/Graphics.h"
#include "Hydra/Engine.h"

#include "Hydra/Render/Pipeline/BindingHelpers.h"

namespace Hydra
{
	Map<String, ConstantBufferInfo> Graphics::_ConstantBuffers;
	Map<String, NVRHI::TextureHandle> Graphics::_RenderViewTargets;

	void Graphics::Destroy()
	{
		ITER(_ConstantBuffers, it)
		{
			Engine::GetRenderInterface()->destroyConstantBuffer(it->second.Handle);
		}

		ITER(_RenderViewTargets, it)
		{
			Engine::GetRenderInterface()->describeTexture(it->second);
		}
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

	NVRHI::ConstantBufferHandle Graphics::CreateConstantBuffer(uint32_t size, const String & mappedName, const unsigned int specificBinding, uint32_t slot)
	{
		if (_ConstantBuffers.find(mappedName) != _ConstantBuffers.end())
		{
			Log("Graphics::CreateConstantBuffer", ToString(size) + ", " + mappedName + ", " + ToString(specificBinding), "Name is already exist!");
			return nullptr;
		}

		ConstantBufferInfo info = {};
		info.Name = mappedName;
		info.DataSize = size;
		info.Slot = slot;
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

	NVRHI::TextureHandle Graphics::CreateRenderTarget(const String & name, const NVRHI::Format::Enum & format, UINT width, UINT height, const NVRHI::Color & clearColor, UINT sampleCount)
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

	NVRHI::TextureHandle Graphics::GetRenderTarget(const String & name)
	{
		if (_RenderViewTargets.find(name) != _RenderViewTargets.end())
		{
			return _RenderViewTargets[name];
		}
		return nullptr;
	}
}