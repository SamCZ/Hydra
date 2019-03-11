#include "Hydra/Render/Shader.h"

#include "Hydra/Render/Pipeline/BindingHelpers.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#include "Hydra/Engine.h"

namespace Hydra
{
	Shader::Shader(const String& name, NVRHI::ShaderHandle shaderHandle, ID3DBlob * shaderBlob) : _Name(name), _ShaderHandle(shaderHandle), _ShaderBlob(shaderBlob), _InputLayoutHandle(nullptr)
	{
		Initialize();
	}

	Shader::~Shader()
	{
		if (_ShaderBlob)
		{
			_ShaderBlob->Release();
		}

		IRendererInterface renderInterface = Engine::GetRenderInterface();

		if (renderInterface)
		{
			renderInterface->destroyShader(_ShaderHandle);
		}
		else
		{
			Log("Shader::~Shader", "Was unsafe deleted !");
			delete _ShaderHandle;
		}

		for (int i = 0; i < _ConstantBufferCount; i++)
		{
			RawShaderConstantBuffer& buffer = _ConstantBuffers[i];
			delete[] buffer.LocalDataBuffer;

			if (renderInterface)
			{
				renderInterface->destroyConstantBuffer(buffer.ConstantBuffer);
			}
			else
			{
				Log("Shader::~Shader", "Constant buffer " + buffer.Name + " unsafe deleted !");
				delete buffer.ConstantBuffer;
			}
		}

		delete[] _ConstantBuffers;

		if (_InputLayoutHandle != nullptr)
		{
			if (renderInterface)
			{
				renderInterface->destroyInputLayout(_InputLayoutHandle);
			}
			else
			{
				Log("Shader::~Shader", "Input layout was unsafe deleted !");
				delete _InputLayoutHandle;
			}
		}
	}

	NVRHI::ShaderHandle Shader::GetHandle()
	{
		return _ShaderHandle;
	}

	ID3DBlob* Shader::GetBlob()
	{
		return _ShaderBlob;
	}

	NVRHI::InputLayoutHandle Shader::GetOrGenerateInputLayout()
	{
		//TODO: Not working properly
		if (_ShaderBlob == nullptr)
		{
			return nullptr;
		}

		if (_InputLayoutHandle != nullptr)
		{
			return _InputLayoutHandle;
		}

		ID3D11ShaderReflection* reflection;
		D3DReflect(_ShaderBlob->GetBufferPointer(), _ShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection);

		D3D11_SHADER_DESC shaderDesc;
		reflection->GetDesc(&shaderDesc);

		if (shaderDesc.InputParameters == 0)
		{
			reflection->Release();
			return nullptr;
		}

		uint32_t nextSize = 0;

		List<NVRHI::VertexAttributeDesc> attributes;
		for (unsigned int i = 0; i < shaderDesc.InputParameters; i++)
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
			reflection->GetInputParameterDesc(i, &paramDesc);

			String perInstanceStr = "_PER_INSTANCE";
			String sem = paramDesc.SemanticName;
			int lenDiff = (int)sem.size() - (int)perInstanceStr.size();
			bool isPerInstance = lenDiff >= 0 && sem.compare(lenDiff, perInstanceStr.size(), perInstanceStr) == 0;

			uint32_t sizeToAdd = 0;
			NVRHI::Format::Enum format = NVRHI::Format::UNKNOWN;

			if (paramDesc.Mask == 1)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) {
					format = NVRHI::Format::R32_UINT;
					sizeToAdd = sizeof(long unsigned int);
				}
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) {
					format = NVRHI::Format::R32_UINT;
					sizeToAdd = sizeof(long unsigned int);
				}
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) {
					format = NVRHI::Format::R32_FLOAT;
					sizeToAdd = sizeof(float);
				}
			}
			else if (paramDesc.Mask <= 3)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				{
					format = NVRHI::Format::RG32_UINT;
					sizeToAdd = sizeof(long unsigned int) * 2;
				}
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				{
					format = NVRHI::Format::RG32_UINT;
					sizeToAdd = sizeof(long unsigned int) * 2;
				}
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				{
					format = NVRHI::Format::RG32_FLOAT;
					sizeToAdd = sizeof(float) * 2;
				}
			}
			else if (paramDesc.Mask <= 7)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				{
					format = NVRHI::Format::RGB32_UINT;
					sizeToAdd = sizeof(long unsigned int) * 3;
				}
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				{
					format = NVRHI::Format::RGB32_UINT;
					sizeToAdd = sizeof(long unsigned int) * 3;
				}
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				{
					format = NVRHI::Format::RGB32_FLOAT;
					sizeToAdd = sizeof(float) * 3;
				}
			}
			else if (paramDesc.Mask <= 15)
			{
				if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				{
					format = NVRHI::Format::RGBA32_UINT;
					sizeToAdd = sizeof(long unsigned int) * 4;
				}
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				{
					format = NVRHI::Format::RGBA32_UINT;
					sizeToAdd = sizeof(long unsigned int) * 4;
				}
				else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				{
					format = NVRHI::Format::RGBA32_FLOAT;
					sizeToAdd = sizeof(float) * 4;
				}
			}

			//paramDesc.SemanticName, attr.semanticIndex, format, isPerInstance ? 1 : 0, nextSize, isPerInstance
			NVRHI::VertexAttributeDesc attr = {  };
			attr.name = paramDesc.SemanticName;
			attr.semanticIndex = attr.semanticIndex;
			attr.format = format;
			attr.bufferIndex = isPerInstance ? 1 : 0;
			attr.offset = nextSize;
			attr.isInstanced = isPerInstance;

			attributes.push_back(attr);

			nextSize += sizeToAdd;
		}

		_InputLayoutHandle = Engine::GetRenderInterface()->createInputLayout(&attributes[0], static_cast<uint32_t>(attributes.size()), _ShaderBlob->GetBufferPointer(), _ShaderBlob->GetBufferSize());

		reflection->Release();

		return _InputLayoutHandle;
	}

	bool Shader::SetVariable(const String & name, const void * data, unsigned int size)
	{
		if (!IsVariableExists(name))
		{
			return false;
		}

		RawShaderVariable& varDef = _Variables[name];

		if (varDef.Size != size)
		{
			return false;
		}

		memcpy(_ConstantBuffers[varDef.ConstantBufferIndex].LocalDataBuffer + varDef.ByteOffset, data, size);

		_ConstantBuffers[varDef.ConstantBufferIndex].MarkUpdate = true;

		return true;
	}

	bool Shader::IsVariableExists(const String & name)
	{
		return _Variables.find(name) != _Variables.end();
	}

	bool Shader::IsVariableExists(const String & name, unsigned int size)
	{
		if (!IsVariableExists(name))
		{
			return false;
		}

		RawShaderVariable& varDef = _Variables[name];

		if (varDef.Size != size)
		{
			return false;
		}

		return true;
	}

	void Shader::UploadVariableData()
	{
		for (int i = 0; i < _ConstantBufferCount; i++)
		{
			if (_ConstantBuffers[i].MarkUpdate)
			{
				Engine::GetRenderInterface()->writeConstantBuffer(_ConstantBuffers[i].ConstantBuffer, _ConstantBuffers[i].LocalDataBuffer, _ConstantBuffers[i].Size);
				_ConstantBuffers[i].MarkUpdate = false;
			}
		}
	}

	void Shader::BindConstantBuffers(NVRHI::PipelineStageBindings& binding)
	{
		for (int i = 0; i < _ConstantBufferCount; i++)
		{
			NVRHI::BindConstantBuffer(binding, _ConstantBuffers[i].BindIndex, _ConstantBuffers[i].ConstantBuffer);
		}
	}

	void Shader::SetTexture(const String& name, NVRHI::TextureHandle texture)
	{
		if (!IsTextureExists(name)) return;

		RawShaderTextureDefine& define = _TextureDefines[name];

		define.TextureHandle = texture;
	}

	void Shader::SetSampler(const String& name, NVRHI::SamplerHandle sampler)
	{
		if (!IsSamplerExists(name)) return;

		RawShaderSamplerDefine& define = _SamplerDefines[name];

		define.SamplerHandle = sampler;
	}

	bool Shader::IsTextureExists(const String & name)
	{
		return _TextureDefines.find(name) != _TextureDefines.end();
	}

	bool Shader::IsSamplerExists(const String & name)
	{
		return _SamplerDefines.find(name) != _SamplerDefines.end();;
	}

	NVRHI::TextureHandle Shader::GetTexture(const String & name)
	{
		if (IsTextureExists(name))
		{
			return _TextureDefines[name].TextureHandle;
		}

		return nullptr;
	}

	NVRHI::SamplerHandle Shader::GetSampler(const String & name)
	{
		if (IsSamplerExists(name))
		{
			return _SamplerDefines[name].SamplerHandle;
		}

		return nullptr;
	}

	void Shader::BindTextures(NVRHI::PipelineStageBindings & binding)
	{
		ITER(_TextureDefines, it)
		{
			NVRHI::BindTexture(binding, it->second.BindIndex, it->second.TextureHandle);
		}
	}

	void Shader::BindSamplers(NVRHI::PipelineStageBindings & binding)
	{
		ITER(_SamplerDefines, it)
		{
			NVRHI::BindSampler(binding, it->second.BindIndex, it->second.SamplerHandle);
		}
	}

	void Shader::BindAllData(NVRHI::PipelineStageBindings & binding)
	{
		BindConstantBuffers(binding);
		BindSamplers(binding);
		BindTextures(binding);
	}

	String Hydra::Shader::GetName()
	{
		return _Name;
	}

	void Shader::Initialize()
	{
		if (_ShaderBlob == nullptr)
		{
			return;
		}

		ID3D11ShaderReflection* reflection;
		D3DReflect(_ShaderBlob->GetBufferPointer(), _ShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection);

		D3D11_SHADER_DESC shaderDesc;
		reflection->GetDesc(&shaderDesc);

		_ConstantBufferCount = shaderDesc.ConstantBuffers;
		_ConstantBuffers = new RawShaderConstantBuffer[_ConstantBufferCount];

		for (int i = 0; i < _ConstantBufferCount; i++)
		{
			ID3D11ShaderReflectionConstantBuffer* cb = reflection->GetConstantBufferByIndex(i);

			D3D11_SHADER_BUFFER_DESC bufferDesc;
			cb->GetDesc(&bufferDesc);

			D3D11_SHADER_INPUT_BIND_DESC bindDesc;
			reflection->GetResourceBindingDescByName(bufferDesc.Name, &bindDesc);

			_ConstantBuffers[i].BindIndex = bindDesc.BindPoint;
			_ConstantBuffers[i].Name = bufferDesc.Name;
			_ConstantBuffers[i].Size = bufferDesc.Size;
			_ConstantBuffers[i].ConstantBuffer = Engine::GetRenderInterface()->createConstantBuffer(NVRHI::ConstantBufferDesc(bufferDesc.Size, bufferDesc.Name), nullptr);
			_ConstantBuffers[i].LocalDataBuffer = new unsigned char[bufferDesc.Size];
			_ConstantBuffers[i].MarkUpdate = false;
			ZeroMemory(_ConstantBuffers[i].LocalDataBuffer, bufferDesc.Size);

			for (unsigned int v = 0; v < bufferDesc.Variables; v++)
			{
				ID3D11ShaderReflectionVariable* var = cb->GetVariableByIndex(v);

				D3D11_SHADER_VARIABLE_DESC varDesc;
				var->GetDesc(&varDesc);

				RawShaderVariable varStruct = {};
				varStruct.ConstantBufferIndex = i;
				varStruct.ByteOffset = varDesc.StartOffset;
				varStruct.Size = varDesc.Size;

				_Variables[varDesc.Name] = varStruct;
			}
		}

		for (unsigned int i = 0; i < shaderDesc.BoundResources; i++)
		{
			D3D11_SHADER_INPUT_BIND_DESC resourceDesc;
			reflection->GetResourceBindingDesc(i, &resourceDesc);

			switch (resourceDesc.Type)
			{
				case D3D_SIT_TEXTURE:
				{
					RawShaderTextureDefine define = {};
					define.BindIndex = resourceDesc.BindPoint;
					define.Index = static_cast<int>(_TextureDefines.size());
					define.TextureHandle = nullptr;

					_TextureDefines[resourceDesc.Name] = define;

					break;
				}
				case D3D_SIT_SAMPLER:
				{
					RawShaderSamplerDefine define = {};
					define.BindIndex = resourceDesc.BindPoint;
					define.Index = static_cast<int>(_SamplerDefines.size());

					_SamplerDefines[resourceDesc.Name] = define;

					break;
				}
			}
		}

		reflection->Release();
	}
}