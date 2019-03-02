#pragma once

#include <d3dcommon.h>

#include "Hydra/Core/SmartPointer.h"
#include "Hydra/Core/Resource.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

namespace Hydra
{
	class Shader : public Resource
	{
	private:
		String _RenderStage;

		NVRHI::ShaderHandle _VertexShader;
		NVRHI::ShaderHandle _HullShader;
		NVRHI::ShaderHandle _DomainShader;
		NVRHI::ShaderHandle _GeometryShader;
		NVRHI::ShaderHandle _PixelShader;
		NVRHI::ShaderHandle _ComputeShader;

		ID3DBlob* _VertexShaderBlob;
		ID3DBlob* _HullShaderBlob;
		ID3DBlob* _DomainShaderBlob;
		ID3DBlob* _GeometryShaderBlob;
		ID3DBlob* _PixelShaderBlob;
		ID3DBlob* _ComputeShaderBlob;

		NVRHI::DrawCallState state;

	public:
		Shader();
		~Shader();

		NVRHI::ShaderHandle GetShader(const NVRHI::ShaderType::Enum& type);

		ID3DBlob* GetShaderBlob(const NVRHI::ShaderType::Enum& type);

		void SetShader(const NVRHI::ShaderType::Enum& type, NVRHI::ShaderHandle shaderHandle, ID3DBlob* blob);

		void SetRenderStage(const String& stage);
		String GetRenderStage();
	};

	typedef SharedPtr<Shader> ShaderPtr;
}