#include "Hydra/Render/Shader.h"

namespace Hydra
{
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
}