#if 0
#include "Hydra/Render/TechniqueOld.h"

#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")

#include "Hydra/Engine.h"

namespace Hydra
{
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
				//OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				std::cout << (char*)errorBlob->GetBufferPointer() << std::endl;
				errorBlob->Release();
			}

			if (shaderBlob)
				shaderBlob->Release();

			return hr;
		}

		*blob = shaderBlob;

		return hr;
	}

	Technique::Technique(const File& source) : _Source(source), _ShaderSource(nullptr)
	{
		SetSource(source.GetPath());

		Recompile();
	}

	Technique::~Technique()
	{
		ITER(_Shaders, it)
		{
			delete it->second;
		}
	}

	void Technique::Recompile(bool loadNewSource)
	{
		//TODO: Runtime recompilations

		if (_ShaderSource == nullptr || loadNewSource)
		{
			if (_ShaderSource != nullptr)
			{
				delete _ShaderSource;
			}

			_ShaderSource = CreateShaderSource();

			ITER(_Shaders, it)
			{
				delete it->second;
			}

			_Shaders.clear();
		}

		IRendererInterface renderer = Engine::GetRenderInterface();

		if (_ShaderSource->ShaderTypes.size() == 0)
		{
			LogError("ShaderImporter::Import", _Source.GetPath(), "ERROR: Has 0 defined shaders !");

			return;
		}

		ITER(_ShaderSource->ShaderTypes, it)
		{
			NVRHI::ShaderType::Enum type = it->first;
			String entryPoint = it->second;

			//Log("ShaderImporter::Import", file.GetPath(), "Found entry point: " + entryPoint);

			D3D_SHADER_MACRO* defines = nullptr;

			if (_ShaderSource->Keywords.find(type) != _ShaderSource->Keywords.end())
			{

				List<String>& keywords = _ShaderSource->Keywords[type];
				List<String> keywordsToEnable;

				for (String& kw : keywords)
				{
					if (_TestHash & _ShaderSource->KeywordIndexMap[kw])
					{
						keywordsToEnable.emplace_back(kw);
					}
				}

				if (keywordsToEnable.size() > 0)
				{
					defines = new D3D_SHADER_MACRO[keywordsToEnable.size()+1];

					int i;

					for (i = 0; i < keywordsToEnable.size(); i++)
					{
						String& kw = keywords[i];

						defines[i] = { kw.c_str(), "1" };
					}

					defines[i] = { NULL, NULL }; // IMPORTANT ! (If not defined function D3DCompile throws an wierd error)
				}

			}

			ID3DBlob* shaderBlob = nullptr;
			HRESULT hr = CompileShaderFromString(_ShaderSource->Source, _Source.GetName(), defines, NULL, entryPoint.c_str(), GetFeatureLevelForShaderType(type).c_str(), &shaderBlob);

			if (defines != nullptr)
			{
				delete[] defines;
			}

			if (FAILED(hr))
			{
				printf("Failed compiling vertex shader (%s) %08X\n", _Source.GetPath().c_str(), hr);
				continue;
			}

			NVRHI::ShaderHandle shaderHandle = renderer->createShader(NVRHI::ShaderDesc(type), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());

			if (shaderHandle != nullptr)
			{
				_Shaders[type] = new Shader(GetSource(), shaderHandle, shaderBlob);
			}
		}

		Log("Technique::Recompile", _Source.GetPath(), "Loaded.");
	}

	void Technique::SetKeywordByHash(uint32 hash)
	{
		_TestHash = hash;
	}

	uint32 Technique::GetKeywordHash(const List<String>& keywords)
	{
		uint32 hash = 0;

		for (const String& kw : keywords)
		{
			if (_ShaderSource->KeywordIndexMap.find(kw) != _ShaderSource->KeywordIndexMap.end())
			{
				hash |= _ShaderSource->KeywordIndexMap[kw];
			}
		}

		return hash;
	}

	ShaderPtr Technique::GetShader(const NVRHI::ShaderType::Enum & type)
	{
		if (_Shaders.find(type) != _Shaders.end())
		{
			return _Shaders[type];
		}

		return nullptr;
	}

	NVRHI::ShaderHandle Technique::GetRawShader(const NVRHI::ShaderType::Enum& type)
	{
		if (_Shaders.find(type) != _Shaders.end())
		{
			return _Shaders[type]->GetHandle();
		}

		return nullptr;
	}

	ID3DBlob * Technique::GetShaderBlob(const NVRHI::ShaderType::Enum& type)
	{
		if (_Shaders.find(type) != _Shaders.end())
		{
			return _Shaders[type]->GetBlob();
		}

		return nullptr;
	}

	ShaderSource* Technique::CreateShaderSource()
	{
		ShaderSource* shaderSource = new ShaderSource();

		int nextKeywordId = 1;

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
									shaderSource->ShaderTypes[shaderType] = paramValue;
								}

								if (paramName == "rs")
								{
									_RenderStage = paramValue;

									//Log("ShaderImporter::Import", file.GetPath(), "Found render stage: " + renderStage);
								}

							}
							else if (paramSplit.size() == 3)
							{
								String paramName = paramSplit[0];
								String paramValue0 = paramSplit[1];
								String paramValue1 = paramSplit[2];

								if (paramName == "kw")
								{
									NVRHI::ShaderType::Enum shaderType = GetShaderTypeByName(paramValue0);
									shaderSource->Keywords[shaderType].push_back(paramValue1);

									std::cout << paramValue0 << " : " << paramValue1 << std::endl;

									shaderSource->KeywordIndexMap[paramValue1] = 1 << nextKeywordId;
									nextKeywordId++;
								}
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
				shaderSource->Source += line + "\r\n";
			}
		}

		return shaderSource;
	}

	NVRHI::ShaderType::Enum Technique::GetShaderTypeByName(const String& name)
	{
		if (name == "vert")
		{
			return NVRHI::ShaderType::SHADER_VERTEX;
		} else if (name == "hull")
		{
			return NVRHI::ShaderType::SHADER_HULL;
		} else if (name == "dom")
		{
			return NVRHI::ShaderType::SHADER_DOMAIN;
		} else if (name == "geom")
		{
			return NVRHI::ShaderType::SHADER_GEOMETRY;
		} else if (name == "pixel")
		{
			return NVRHI::ShaderType::SHADER_PIXEL;
		} else if (name == "cmp")
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
}
#endif