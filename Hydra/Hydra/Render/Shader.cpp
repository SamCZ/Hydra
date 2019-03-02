#include "Hydra/Render/Shader.h"

#include "Hydra/Engine.h"

namespace Hydra
{
	Shader::Shader()
		: _VertexShader(nullptr),     _HullShader(nullptr),     _DomainShader(nullptr),     _GeometryShader(nullptr),     _PixelShader(nullptr),     _ComputeShader(nullptr),
		  _VertexShaderBlob(nullptr), _HullShaderBlob(nullptr), _DomainShaderBlob(nullptr), _GeometryShaderBlob(nullptr), _PixelShaderBlob(nullptr), _ComputeShaderBlob(nullptr)
	{ }

	Shader::~Shader()
	{
		if (_VertexShaderBlob)
		{
			_VertexShaderBlob->Release();
			_VertexShaderBlob = nullptr;
		}

		if (_HullShaderBlob)
		{
			_HullShaderBlob->Release();
			_HullShaderBlob = nullptr;
		}

		if (_DomainShaderBlob)
		{
			_DomainShaderBlob->Release();
			_DomainShaderBlob = nullptr;
		}

		if (_GeometryShaderBlob)
		{
			_GeometryShaderBlob->Release();
			_GeometryShaderBlob = nullptr;
		}

		if (_PixelShaderBlob)
		{
			_PixelShaderBlob->Release();
			_PixelShaderBlob = nullptr;
		}

		if (_ComputeShaderBlob)
		{
			_ComputeShaderBlob->Release();
			_ComputeShaderBlob = nullptr;
		}

		IRendererInterface renderInterface = Engine::GetRenderInterface();

		if (renderInterface)
		{
			if (_VertexShader)
			{
				renderInterface->destroyShader(_VertexShader);
				_VertexShader = nullptr;
			}

			if (_HullShader)
			{
				renderInterface->destroyShader(_HullShader);
				_HullShader = nullptr;
			}

			if (_DomainShader)
			{
				renderInterface->destroyShader(_DomainShader);
				_DomainShader = nullptr;
			}

			if (_GeometryShader)
			{
				renderInterface->destroyShader(_GeometryShader);
				_GeometryShader = nullptr;
			}

			if (_PixelShader)
			{
				renderInterface->destroyShader(_PixelShader);
				_PixelShader = nullptr;
			}

			if (_ComputeShader)
			{
				renderInterface->destroyShader(_ComputeShader);
				_ComputeShader = nullptr;
			}
		}
	}

	NVRHI::ShaderHandle Shader::GetShader(const NVRHI::ShaderType::Enum& type)
	{
		switch (type)
		{
		case NVRHI::ShaderType::SHADER_VERTEX:
			return _VertexShader;
			break;
		case NVRHI::ShaderType::SHADER_HULL:
			return _HullShader;
			break;
		case NVRHI::ShaderType::SHADER_DOMAIN:
			return _DomainShader;
			break;
		case NVRHI::ShaderType::SHADER_GEOMETRY:
			return _GeometryShader;
			break;
		case NVRHI::ShaderType::SHADER_PIXEL:
			return _PixelShader;
			break;
		case NVRHI::ShaderType::SHADER_COMPUTE:
			return _ComputeShader;
			break;
		default:
			return nullptr;
		}
	}

	ID3DBlob * Shader::GetShaderBlob(const NVRHI::ShaderType::Enum & type)
	{
		switch (type)
		{
		case NVRHI::ShaderType::SHADER_VERTEX:
			return _VertexShaderBlob;
			break;
		case NVRHI::ShaderType::SHADER_HULL:
			return _HullShaderBlob;
			break;
		case NVRHI::ShaderType::SHADER_DOMAIN:
			return _DomainShaderBlob;
			break;
		case NVRHI::ShaderType::SHADER_GEOMETRY:
			return _GeometryShaderBlob;
			break;
		case NVRHI::ShaderType::SHADER_PIXEL:
			return _PixelShaderBlob;
			break;
		case NVRHI::ShaderType::SHADER_COMPUTE:
			return _ComputeShaderBlob;
			break;
		default:
			return nullptr;
		}
	}

	void Shader::SetShader(const NVRHI::ShaderType::Enum & type, NVRHI::ShaderHandle shaderHandle, ID3DBlob* blob)
	{
		switch (type)
		{
		case NVRHI::ShaderType::SHADER_VERTEX:
			_VertexShader = shaderHandle;
			_VertexShaderBlob = blob;
			break;
		case NVRHI::ShaderType::SHADER_HULL:
			_HullShader = shaderHandle;
			_HullShaderBlob = blob;
			break;
		case NVRHI::ShaderType::SHADER_DOMAIN:
			_DomainShader = shaderHandle;
			_DomainShaderBlob = blob;
			break;
		case NVRHI::ShaderType::SHADER_GEOMETRY:
			_GeometryShader = shaderHandle;
			_GeometryShaderBlob = blob;
			break;
		case NVRHI::ShaderType::SHADER_PIXEL:
			_PixelShader = shaderHandle;
			_PixelShaderBlob = blob;
			break;
		case NVRHI::ShaderType::SHADER_COMPUTE:
			_ComputeShader = shaderHandle;
			_ComputeShaderBlob = blob;
			break;
		}
	}

	void Shader::SetRenderStage(const String & stage)
	{
		_RenderStage = stage;
	}

	String Shader::GetRenderStage()
	{
		return _RenderStage;
	}
}