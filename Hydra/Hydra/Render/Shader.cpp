#include "Hydra/Render/Shader.h"

#include "Hydra/Engine.h"

#include <d3d11.h>
#include <d3dcompiler.h>

namespace Hydra
{
	Shader::Shader(const String& name, const NVRHI::ShaderType::Enum& type, NVRHI::ShaderHandle shaderHandle, ID3DBlob* shaderBlob)
		: _Name(name), _Type(type), _Handle(shaderHandle), _Blob(shaderBlob)
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

	void Shader::Initialize()
	{
		if (_Blob == nullptr)
		{
			return;
		}

		ID3D11ShaderReflection* reflection;
		D3DReflect(_Blob->GetBufferPointer(), _Blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection);

		D3D11_SHADER_DESC shaderDesc;
		reflection->GetDesc(&shaderDesc);



		reflection->Release();
	}
}