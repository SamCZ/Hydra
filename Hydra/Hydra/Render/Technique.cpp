#include "Hydra/Render/Technique.h"

#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")

#include "Hydra/Engine.h"

namespace Hydra
{
	HRESULT CompileShaderFromString(_In_ const String& shaderSource, _In_ const String& name, _In_ ID3DInclude* include, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
	{
		if (!entryPoint || !profile || !blob)
			return E_INVALIDARG;

		*blob = nullptr;

		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG ) || true
		flags |= D3DCOMPILE_DEBUG;
#endif

		//const D3D_SHADER_MACRO defines[0];

		ID3DBlob* shaderBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		HRESULT hr = D3DCompile(shaderSource.c_str(), shaderSource.length(), name.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0, &shaderBlob, &errorBlob);
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

	Technique::Technique(const File& source) : _Source(source)
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

		String shaderSource;
		Map<NVRHI::ShaderType::Enum, String> shaderTypes;

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

								if (paramName == "vert")
								{
									shaderTypes[NVRHI::ShaderType::SHADER_VERTEX] = paramValue;
								}

								if (paramName == "hull")
								{
									shaderTypes[NVRHI::ShaderType::SHADER_HULL] = paramValue;
								}

								if (paramName == "dom")
								{
									shaderTypes[NVRHI::ShaderType::SHADER_DOMAIN] = paramValue;
								}

								if (paramName == "geom")
								{
									shaderTypes[NVRHI::ShaderType::SHADER_GEOMETRY] = paramValue;
								}

								if (paramName == "pixel")
								{
									shaderTypes[NVRHI::ShaderType::SHADER_PIXEL] = paramValue;
								}

								if (paramName == "cmp")
								{
									shaderTypes[NVRHI::ShaderType::SHADER_COMPUTE] = paramValue;
								}

								if (paramName == "rs")
								{
									_RenderStage = paramValue;

									//Log("ShaderImporter::Import", file.GetPath(), "Found render stage: " + renderStage);
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
				shaderSource += line + "\r\n";
			}
		}

		IRendererInterface renderer = Engine::GetRenderInterface();

		if (shaderTypes.size() == 0)
		{
			LogError("ShaderImporter::Import", _Source.GetPath(), "ERROR: Has 0 defined shaders !");

			return;
		}

		ITER(shaderTypes, it)
		{
			NVRHI::ShaderType::Enum type = it->first;
			String entryPoint = it->second;

			//Log("ShaderImporter::Import", file.GetPath(), "Found entry point: " + entryPoint);

			ID3DBlob* shaderBlob = nullptr;
			HRESULT hr = CompileShaderFromString(shaderSource, _Source.GetName(), NULL, entryPoint.c_str(), GetFeatureLevelForShaderType(type).c_str(), &shaderBlob);

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

	ShaderPtr Hydra::Technique::GetShader(const NVRHI::ShaderType::Enum & type)
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