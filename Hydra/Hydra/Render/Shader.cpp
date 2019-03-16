#include "Hydra/Render/Shader.h"

#include "Hydra/Engine.h"

#include <d3d11.h>
#include <d3dcompiler.h>

namespace Hydra
{
	Shader::Shader(const String& name, const NVRHI::ShaderType::Enum& type, NVRHI::ShaderHandle shaderHandle, ID3DBlob* shaderBlob)
		: _Name(name), _Type(type), _Handle(shaderHandle), _Blob(shaderBlob), _LocalShaderVarCache(nullptr)
	{
		Initialize();
	}

	Shader::~Shader()
	{
		if (_Blob)
		{
			_Blob->Release();
			_Blob = nullptr;
		}

		if (_Handle)
		{
			Engine::GetRenderInterface()->destroyShader(_Handle);
			_Handle = nullptr;
		}

		delete _LocalShaderVarCache;
	}

	NVRHI::ShaderHandle Shader::GetRaw()
	{
		return _Handle;
	}

	ID3DBlob* Shader::GetBlob()
	{
		return _Blob;
	}

	NVRHI::ShaderType::Enum Shader::GetType()
	{
		return _Type;
	}

	ShaderVars* Shader::CreateShaderVars()
	{
		if (_LocalShaderVarCache == nullptr)
		{
			Initialize();
		}

		return new ShaderVars(*_LocalShaderVarCache);
	}

	void Shader::Initialize()
	{
		if (_Blob == nullptr)
		{
			return;
		}

		_LocalShaderVarCache = new ShaderVars();

		_LocalShaderVarCache->ShaderType = _Type;

		ID3D11ShaderReflection* reflection;
		D3DReflect(_Blob->GetBufferPointer(), _Blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection);

		D3D11_SHADER_DESC shaderDesc;
		reflection->GetDesc(&shaderDesc);

		_LocalShaderVarCache->ConstantBufferCount = shaderDesc.ConstantBuffers;
		_LocalShaderVarCache->ConstantBuffers = new RawShaderConstantBuffer[_LocalShaderVarCache->ConstantBufferCount];

		for (int i = 0; i < _LocalShaderVarCache->ConstantBufferCount; i++)
		{
			ID3D11ShaderReflectionConstantBuffer* cb = reflection->GetConstantBufferByIndex(i);

			D3D11_SHADER_BUFFER_DESC bufferDesc;
			cb->GetDesc(&bufferDesc);

			D3D11_SHADER_INPUT_BIND_DESC bindDesc;
			reflection->GetResourceBindingDescByName(bufferDesc.Name, &bindDesc);

			_LocalShaderVarCache->ConstantBuffers[i].BindIndex = bindDesc.BindPoint;
			_LocalShaderVarCache->ConstantBuffers[i].Name = bufferDesc.Name;
			_LocalShaderVarCache->ConstantBuffers[i].Size = bufferDesc.Size;
			_LocalShaderVarCache->ConstantBuffers[i].MarkUpdate = false;

			for (unsigned int v = 0; v < bufferDesc.Variables; v++)
			{
				ID3D11ShaderReflectionVariable* var = cb->GetVariableByIndex(v);

				D3D11_SHADER_VARIABLE_DESC varDesc;
				var->GetDesc(&varDesc);

				RawShaderVariable varStruct = {};
				varStruct.ConstantBufferIndex = i;
				varStruct.ByteOffset = varDesc.StartOffset;
				varStruct.Size = varDesc.Size;

				_LocalShaderVarCache->Variables[varDesc.Name] = varStruct;
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
					define.Index = static_cast<int>(_LocalShaderVarCache->TextureDefines.size());
					define.TextureHandle = nullptr;

					_LocalShaderVarCache->TextureDefines[resourceDesc.Name] = define;

					break;
				}
				case D3D_SIT_SAMPLER:
				{
					RawShaderSamplerDefine define = {};
					define.BindIndex = resourceDesc.BindPoint;
					define.Index = static_cast<int>(_LocalShaderVarCache->SamplerDefines.size());

					_LocalShaderVarCache->SamplerDefines[resourceDesc.Name] = define;

					break;
				}
			}
		}

		reflection->Release();
	}
}