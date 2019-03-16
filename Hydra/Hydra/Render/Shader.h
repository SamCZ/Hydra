#pragma once

#include "Hydra/Core/String.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"
#include <d3d11.h>

namespace Hydra
{
	class Shader
	{
	private:
		String _Name;
		NVRHI::ShaderType::Enum _Type;
		NVRHI::ShaderHandle _Handle;
		ID3DBlob* _Blob;

	public:
		Shader(const String& name, const NVRHI::ShaderType::Enum& type, NVRHI::ShaderHandle shaderHandle, ID3DBlob* shaderBlob);
		~Shader();

		NVRHI::ShaderHandle GetRaw();
		ID3DBlob* GetBlob();
		
		NVRHI::ShaderType::Enum GetType();

	private:
		void Initialize();
	};
}