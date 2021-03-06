#include "Hydra/Render/Shader.h"

#include "Hydra/EngineContext.h"

#include <d3d11.h>
#include <d3dcompiler.h>

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
		// Handle deleted in Technique
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

DXGI_FORMAT GetDXGIFormat(D3D11_SIGNATURE_PARAMETER_DESC& pd)
{
	BYTE mask = pd.Mask;
	int varCount = 0;
	while (mask)
	{
		if (mask & 0x01) varCount++;
		mask = mask >> 1;
	}

	if (pd.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
	{
		if (varCount == 4) return DXGI_FORMAT_R32G32B32A32_FLOAT;
		else if (varCount == 3) return DXGI_FORMAT_R32G32B32_FLOAT;
		else if (varCount == 2) return DXGI_FORMAT_R32G32_FLOAT;
		else if (varCount == 1) return DXGI_FORMAT_R32_FLOAT;
	}
	else if (pd.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
	{
		if (varCount == 4) return DXGI_FORMAT_R32G32B32A32_SINT;
		else if (varCount == 3) return DXGI_FORMAT_R32G32B32_SINT;
		else if (varCount == 2) return DXGI_FORMAT_R32G32_SINT;
		else if (varCount == 1) return DXGI_FORMAT_R32_SINT;
	}
	else if (pd.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
	{
		if (varCount == 4) return DXGI_FORMAT_R32G32B32A32_UINT;
		else if (varCount == 3) return DXGI_FORMAT_R32G32B32_UINT;
		else if (varCount == 2) return DXGI_FORMAT_R32G32_UINT;
		else if (varCount == 1) return DXGI_FORMAT_R32_UINT;
	}
	else if (pd.ComponentType == D3D_REGISTER_COMPONENT_UNKNOWN)
	{
	}
	else
	{
	}

	return DXGI_FORMAT_UNKNOWN;
}

List<ShaderVertexInputDefinition> Shader::GetInputLayoutDefinitions(NVRHI::IRendererInterface* renderInterface)
{
	List<ShaderVertexInputDefinition> definitions;

	if (_Blob == nullptr || _Handle == nullptr || _Type != NVRHI::ShaderType::SHADER_VERTEX)
	{
		return definitions;
	}

	ID3D11ShaderReflection* reflection;
	D3DReflect(_Blob->GetBufferPointer(), _Blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection);

	D3D11_SHADER_DESC shaderDesc;
	reflection->GetDesc(&shaderDesc);

	if (shaderDesc.InputParameters == 0)
	{
		reflection->Release();
		return definitions;
	}

	for (unsigned int i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		reflection->GetInputParameterDesc(i, &paramDesc);

		if (paramDesc.SystemValueType != D3D_NAME_UNDEFINED) continue;

		String perInstanceStr = "_PER_INSTANCE";
		String sem = paramDesc.SemanticName;
		uint8 semanticIndex = paramDesc.SemanticIndex;
		int lenDiff = (int)sem.size() - (int)perInstanceStr.size();
		bool isPerInstance = lenDiff >= 0 && sem.compare(lenDiff, perInstanceStr.size(), perInstanceStr) == 0;

		DXGI_FORMAT format = GetDXGIFormat(paramDesc);

		ShaderVertexInputDefinition def = {};
		def.SemanticName = paramDesc.SemanticName;
		def.SemanticIndex = semanticIndex;
		def.Format = renderInterface->GetFormatFromDXGI(format);
		def.Instanced = isPerInstance;

		definitions.emplace_back(def);
	}

	Log(ToString(shaderDesc.InputParameters));

	reflection->Release();

	return definitions;
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


	int constantBufferRealCount = 0;

	for (unsigned int i = 0; i < shaderDesc.ConstantBuffers; i++)
	{
		ID3D11ShaderReflectionConstantBuffer* cb = reflection->GetConstantBufferByIndex(i);

		D3D11_SHADER_BUFFER_DESC bufferDesc;
		cb->GetDesc(&bufferDesc);

		D3D11_SHADER_INPUT_BIND_DESC bindDesc;
		reflection->GetResourceBindingDescByName(bufferDesc.Name, &bindDesc);

		if (_Type == NVRHI::ShaderType::SHADER_COMPUTE)
		{
			//std::cout << "yo" << std::endl;
		}

		if (bindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED || bindDesc.Type == D3D_SIT_STRUCTURED)
		{
			RawShaderBuffer define = {};
			define.Name = bufferDesc.Name;
			define.Size = bufferDesc.Size;
			define.BindIndex = bindDesc.BindPoint;
			define.Index = static_cast<int>(_LocalShaderVarCache->TextureDefines.size());
			define.IsWritable = bindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED;
			define.Buffer = nullptr;

			_LocalShaderVarCache->BufferDefines[bufferDesc.Name] = define;
		}
		else
		{
			constantBufferRealCount++;
		}
	}


	_LocalShaderVarCache->ConstantBufferCount = constantBufferRealCount;
	_LocalShaderVarCache->ConstantBuffers = new RawShaderConstantBuffer[_LocalShaderVarCache->ConstantBufferCount];

	int constantBufferIndex = 0;

	for (int i = 0; i < _LocalShaderVarCache->ConstantBufferCount; i++)
	{
		ID3D11ShaderReflectionConstantBuffer* cb = reflection->GetConstantBufferByIndex(i);

		D3D11_SHADER_BUFFER_DESC bufferDesc;
		cb->GetDesc(&bufferDesc);

		D3D11_SHADER_INPUT_BIND_DESC bindDesc;
		reflection->GetResourceBindingDescByName(bufferDesc.Name, &bindDesc);

		if (bindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED)
		{
			continue;
		}

		_LocalShaderVarCache->ConstantBuffers[constantBufferIndex].BindIndex = bindDesc.BindPoint;
		_LocalShaderVarCache->ConstantBuffers[constantBufferIndex].Name = bufferDesc.Name;
		_LocalShaderVarCache->ConstantBuffers[constantBufferIndex].Size = bufferDesc.Size;
		_LocalShaderVarCache->ConstantBuffers[constantBufferIndex].MarkUpdate = false;

		for (unsigned int v = 0; v < bufferDesc.Variables; v++)
		{
			ID3D11ShaderReflectionVariable* var = cb->GetVariableByIndex(v);
			ID3D11ShaderReflectionType* type = var->GetType();

			D3D11_SHADER_VARIABLE_DESC varDesc;
			var->GetDesc(&varDesc);

			RawShaderVariable varStruct = {};
			varStruct.ConstantBufferIndex = constantBufferIndex;
			varStruct.ByteOffset = varDesc.StartOffset;
			varStruct.Size = varDesc.Size;

			D3D11_SHADER_TYPE_DESC typeDesc;
			type->GetDesc(&typeDesc);

			String typeName = typeDesc.Name;

			if (typeName == "int")
			{
				_LocalShaderVarCache->VariableTypes[varDesc.Name] = VarType::Int;
			}
			else if (typeName == "float")
			{
				_LocalShaderVarCache->VariableTypes[varDesc.Name] = VarType::Float;
			}
			else if (typeName == "float2")
			{
				_LocalShaderVarCache->VariableTypes[varDesc.Name] = VarType::Vector2;
			}
			else if (typeName == "float3")
			{
				_LocalShaderVarCache->VariableTypes[varDesc.Name] = VarType::Vector3;
			}
			else if (typeName == "float4")
			{
				_LocalShaderVarCache->VariableTypes[varDesc.Name] = VarType::Vector4;
			}
			else if (typeName == "bool")
			{
				_LocalShaderVarCache->VariableTypes[varDesc.Name] = VarType::Bool;
			}

			/*for (unsigned j = 0; j < typeDesc.Members; ++j)
			{
				ID3D11ShaderReflectionType* memberType = type->GetMemberTypeByIndex(j);
				D3D11_SHADER_TYPE_DESC memberTypeDesc;
				memberType->GetDesc(&memberTypeDesc);
			}*/

			_LocalShaderVarCache->Variables[varDesc.Name] = varStruct;
		}

		constantBufferIndex++;
	}

	if (_Type == NVRHI::ShaderType::SHADER_COMPUTE)
	{
		uint32_t groupsX;
		uint32_t groupsY;
		uint32_t groupsZ;

		UINT status = reflection->GetThreadGroupSize(&groupsX, &groupsY, &groupsZ);

		std::cout << "Compute " << groupsX << ", " << groupsY << ", " << groupsZ << std::endl;
	}

	for (unsigned int i = 0; i < shaderDesc.BoundResources; i++)
	{
		D3D11_SHADER_INPUT_BIND_DESC resourceDesc;
		reflection->GetResourceBindingDesc(i, &resourceDesc);

		if (_Type == NVRHI::ShaderType::SHADER_COMPUTE)
		{
			//std::cout << "yo" << std::endl;
		}

		switch (resourceDesc.Type)
		{
		case D3D_SIT_UAV_RWTYPED:
		case D3D_SIT_TEXTURE:
		{
			RawShaderTextureDefine define = {};
			define.BindIndex = resourceDesc.BindPoint;
			define.Index = static_cast<int>(_LocalShaderVarCache->TextureDefines.size());
			define.TextureHandle = nullptr;
			define.IsWritable = resourceDesc.Type == D3D_SIT_UAV_RWTYPED;

			_LocalShaderVarCache->TextureDefines[resourceDesc.Name] = define;

			break;
		}

		case D3D_SIT_UAV_RWSTRUCTURED:
		{
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