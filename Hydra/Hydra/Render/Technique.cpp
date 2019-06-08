#include "Hydra/Render/Technique.h"

#include "Hydra/Core/Log.h"

#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")

#include "Hydra/EngineContext.h"

#include "Hydra/Render/Shader.h"

HRESULT CompileShaderFromString(_In_ const String& shaderSource, _In_ const String& name, _In_ const D3D_SHADER_MACRO* macros, _In_ ID3DInclude* include, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
{
	if (!entryPoint || !profile || !blob)
		return E_INVALIDARG;

	*blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompile(shaderSource.c_str(), shaderSource.length(), name.c_str(), macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0, &shaderBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			LogError(String((char*)errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}

		if (shaderBlob)
			shaderBlob->Release();

		return hr;
	}

	*blob = shaderBlob;

	return hr;
}

Technique::Technique(EngineContext* context, const File& file, bool precompile) : _Context(context), _Source(file), _Precompile(precompile), _NextDefineId(0), _HasInputLayoutID(false), _CanCreateInputLayoutID(false), _VertexShaderInternal(nullptr)
{
	ReadShaderSource();
}

Technique::~Technique()
{
	ITER(_VaryingShaders, it)
	{
		for (Shader* shader : it->second)
		{
			NVRHI::ShaderHandle sHandle = shader->GetRaw();

			if (sHandle)
			{
				_Context->GetRenderInterface()->destroyShader(sHandle);
			}

			delete shader;
		}
	}

	_VaryingShaders.clear();
}

uint32 Technique::GetDefinesHash(Map<String, String>& defines)
{
	uint32 hash = 0;

	for (Map<String, String>::iterator it = defines.begin(); it != defines.end(); it++)
	{
		hash |= GetDefineHash(it->first, it->second);
	}

	return hash;
}

uint32 Technique::GetDefineHash(const String& define, const String& value)
{
	String fullDef = define + "#" + value;
	if (_DefineHashes.find(fullDef) != _DefineHashes.end())
	{
		return _DefineHashes[fullDef];
	}
	else
	{
		int id = _NextDefineId++;

		_DefineHashes[fullDef] = 1 << id;

		return id;
	}
}

void Technique::UpdateInputLayoutID(Map<String, uint32>& hashMap, uint32& maxIndex)
{
	if (_CanCreateInputLayoutID == false)
	{
		return;
	}

	if (!_HasInputLayoutID)
	{
		ITER(_VaryingShaders, it)
		{
			for (Shader* shader : it->second)
			{
				if (shader->GetType() == NVRHI::ShaderType::SHADER_VERTEX)
				{
					_ShaderVertexInputDefinitons = shader->GetInputLayoutDefinitions();

					String hash;

					for (ShaderVertexInputDefinition& definition : _ShaderVertexInputDefinitons)
					{
						hash += definition.ToHash();
					}

					auto iter = hashMap.find(hash);

					if (iter != hashMap.end())
					{
						_InputLayoutID = iter->second;
					}
					else
					{
						_InputLayoutID = maxIndex++;
						hashMap[hash] = _InputLayoutID;
					}

					_HasInputLayoutID = true;

					return;
				}
			}
		}
	}
}

bool Technique::GetInputLayoutID(uint32& out_ID) const
{
	if (_HasInputLayoutID)
	{
		out_ID = _InputLayoutID;

		return true;
	}

	return false;
}

NVRHI::InputLayoutHandle Technique::CreateInputLayout()
{
	if (_HasInputLayoutID == false)
	{
		return nullptr;
	}

	int attrCount = _ShaderVertexInputDefinitons.size();

	NVRHI::VertexAttributeDesc* vertexAttrDecs = new NVRHI::VertexAttributeDesc[attrCount];


	for (int i = 0; i < attrCount; i++)
	{
		ShaderVertexInputDefinition& definition = _ShaderVertexInputDefinitons[i];
		NVRHI::VertexAttributeDesc& attributeDesc = vertexAttrDecs[i];

		attributeDesc.bufferIndex = definition.Instanced ? 1 : 0;
		attributeDesc.format = definition.Format;
		//attributeDesc.

	}

	IRendererInterface renderInterface = _Context->GetRenderInterface();

	ID3DBlob* dataBlob = _VertexShaderInternal->GetBlob();

	return renderInterface->createInputLayout(vertexAttrDecs, attrCount, dataBlob->GetBufferPointer(), dataBlob->GetBufferSize());
}

List<Shader*>& Technique::GetShaders(Map<String, String>& defines, bool recompile)
{
	uint32 defineHash = GetDefinesHash(defines);

	if (_VaryingShaders.find(defineHash) != _VaryingShaders.end() && !recompile)
	{
		return _VaryingShaders[defineHash];
	}
	else
	{
		Log("Compiling shader", _Source.GetPath());

		if (_VaryingShaders.find(defineHash) != _VaryingShaders.end())
		{
			for (Shader* shader : _VaryingShaders[defineHash])
			{
				delete shader;
			}

			_VaryingShaders[defineHash].clear();
		}

		ITER(_ShaderTypes, it)
		{
			NVRHI::ShaderType::Enum type = it->first;
			String entryPoint = it->second;

			D3D_SHADER_MACRO* macros = new D3D_SHADER_MACRO[defines.size() + 1];

			int i = 0;

			for (Map<String, String>::iterator it = defines.begin(); it != defines.end(); it++)
			{
				macros[i++] = { it->first.c_str(), it->second.c_str() };
			}

			macros[i] = { NULL, NULL }; // IMPORTANT ! (If not defined function D3DCompile throws an wierd error)

			ID3DBlob* shaderBlob = nullptr;
			HRESULT hr = CompileShaderFromString(_ShaderCode, _Source.GetName(), macros, NULL, entryPoint.c_str(), GetFeatureLevelForShaderType(type).c_str(), &shaderBlob);

			if (FAILED(hr))
			{
				printf("Failed compiling vertex shader (%s) %08X\n", _Source.GetPath().c_str(), hr);
				continue;
			}

			NVRHI::ShaderHandle shaderHandle = _Context->GetRenderInterface()->createShader(NVRHI::ShaderDesc(type), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());

			if (shaderHandle != nullptr)
			{
				Shader* shader = new Shader(_Source.GetName(), type, shaderHandle, shaderBlob);

				if (type == NVRHI::ShaderType::SHADER_VERTEX)
				{
					_CanCreateInputLayoutID = true;

					_VertexShaderInternal = shader;
				}

				_VaryingShaders[defineHash].emplace_back(shader);
			}
		};

		return _VaryingShaders[defineHash];
	}
}

bool Technique::IsPrecompiled() const
{
	return _Precompile;
}

EngineContext* Technique::GetEngineContext()
{
	return _Context;
}

void Technique::ReadShaderSource()
{
	_ShaderTypes.clear();
	_ShaderCode = String_None;

	List<String> shaderLines = _Source.ReadLines();
	for (String& line : shaderLines)
	{
		if (StartsWith(line, "#pragma hydra"))
		{
			String pragmaData = line.substr(13);
			if (pragmaData.length() > 0)
			{
				if (pragmaData[0] == ' ')
				{
					pragmaData = pragmaData.substr(1);
				}

				List<String> pragmaParams = SplitString(pragmaData, ' ');

				for (String pragmaParam : pragmaParams)
				{
					if (StringContains(pragmaParam, ':'))
					{
						List<String> paramSplit = SplitString(pragmaParam, ':');

						if (paramSplit.size() == 2)
						{
							String paramName = paramSplit[0];
							String paramValue = paramSplit[1];

							NVRHI::ShaderType::Enum shaderType = GetShaderTypeByName(paramName);

							if (shaderType != NVRHI::ShaderType::GRAPHIC_SHADERS_NUM)
							{
								_ShaderTypes[shaderType] = paramValue;
							}

							if (paramName == "rs")
							{
								//_RenderStage = paramValue;

								//Log("ShaderImporter::Import", file.GetPath(), "Found render stage: " + renderStage);
							}

						}
						else if (paramSplit.size() == 3)
						{
							String paramName = paramSplit[0];
							String paramValue0 = paramSplit[1];
							String paramValue1 = paramSplit[2];

							/*if (paramName == "kw")
							{
								NVRHI::ShaderType::Enum shaderType = GetShaderTypeByName(paramValue0);
								shaderSource->Keywords[shaderType].push_back(paramValue1);

								std::cout << paramValue0 << " : " << paramValue1 << std::endl;

								shaderSource->KeywordIndexMap[paramValue1] = 1 << nextKeywordId;
								nextKeywordId++;
							}*/
						}
					}
					else
					{
						// Do other paraments
					}
				}
			}
		}
		else
		{
			_ShaderCode += line + "\r\n";
		}
	}
}

NVRHI::ShaderType::Enum Technique::GetShaderTypeByName(const String& name)
{
	if (name == "vert")
	{
		return NVRHI::ShaderType::SHADER_VERTEX;
	}
	else if (name == "hull")
	{
		return NVRHI::ShaderType::SHADER_HULL;
	}
	else if (name == "dom")
	{
		return NVRHI::ShaderType::SHADER_DOMAIN;
	}
	else if (name == "geom")
	{
		return NVRHI::ShaderType::SHADER_GEOMETRY;
	}
	else if (name == "pixel")
	{
		return NVRHI::ShaderType::SHADER_PIXEL;
	}
	else if (name == "cmp")
	{
		return NVRHI::ShaderType::SHADER_COMPUTE;
	}
	else
	{
		return NVRHI::ShaderType::GRAPHIC_SHADERS_NUM;
	}
}

String Technique::GetFeatureLevelForShaderType(const NVRHI::ShaderType::Enum & type)
{
	switch (type)
	{
	case NVRHI::ShaderType::SHADER_VERTEX:
		return "vs_5_0";
		break;
	case NVRHI::ShaderType::SHADER_HULL:
		return "hs_5_0";
		break;
	case NVRHI::ShaderType::SHADER_DOMAIN:
		return "ds_5_0";
		break;
	case NVRHI::ShaderType::SHADER_GEOMETRY:
		return "gs_5_0";
		break;
	case NVRHI::ShaderType::SHADER_PIXEL:
		return "ps_5_0";
		break;
	case NVRHI::ShaderType::SHADER_COMPUTE:
		return "cs_5_0";
		break;
	default:
		return String();
	}
}