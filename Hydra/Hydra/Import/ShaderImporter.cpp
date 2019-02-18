#include "ShaderImporter.h"

#include <iostream>

#include "Hydra/Core/File.h"
#include "Hydra/Render/Shader.h"

#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")

#include "Hydra/Engine.h"

namespace Hydra
{
	HRESULT CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
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
		HRESULT hr = D3DCompileFromFile(srcFile, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, profile, flags, 0, &shaderBlob, &errorBlob);
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

	/*std::wstring s2ws(const std::string& s)
	{
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}*/

	Shader* ShaderImporter::Import(const File& file)
	{
		wchar_t wchTitle[256];
		MultiByteToWideChar(CP_ACP, 0, file.GetPath().c_str(), -1, wchTitle, 256);

		LPCWSTR path = wchTitle;

		ID3DBlob *vsBlob = nullptr;
		HRESULT hr = CompileShader(path, "MainVS", "vs_5_0", &vsBlob);
		if (FAILED(hr))
		{
			printf("Failed compiling vertex shader %08X\n", hr);
		}

		ID3DBlob *psBlob = nullptr;
		hr = CompileShader(path, "MainPS", "ps_5_0", &psBlob);
		if (FAILED(hr))
		{
			printf("Failed compiling pixel shader %08X\n", hr);
		}

		IRendererInterface renderer = Engine::GetRenderInterface();

		if (vsBlob == nullptr || psBlob == nullptr)
		{
			return nullptr;
		}

		NVRHI::ShaderHandle vertexShader = renderer->createShader(NVRHI::ShaderDesc(NVRHI::ShaderType::SHADER_VERTEX), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize());
		NVRHI::ShaderHandle pixelShader = renderer->createShader(NVRHI::ShaderDesc(NVRHI::ShaderType::SHADER_PIXEL), psBlob->GetBufferPointer(), psBlob->GetBufferSize());

		Shader* shader = new Shader();
		shader->SetSource(file.GetPath());

		shader->SetShader(NVRHI::ShaderType::SHADER_VERTEX, vertexShader, vsBlob);
		shader->SetShader(NVRHI::ShaderType::SHADER_PIXEL, pixelShader, psBlob);

		return shader;
	}
}
