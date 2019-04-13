#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Core/File.h"

#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

namespace Hydra
{
	class Shader;
	class EngineContext;

	class HYDRA_API Technique
	{
	private:
		EngineContext* _Context;

		File _Source;
		bool _Precompile;

		Map<String, uint32> _DefineHashes;
		int _NextDefineId;

		Map<NVRHI::ShaderType::Enum, String> _ShaderTypes;
		String _ShaderCode;

		Map<uint32, List<Shader*>> _VaryingShaders;
	public:
		Technique(EngineContext* context, const File& file, bool precompile);
		~Technique();

		uint32 GetDefinesHash(Map<String, String>& defines);
		uint32 GetDefineHash(const String& define, const String& value);

		List<Shader*>& GetShaders(Map<String, String>& defines, bool recompile);

		bool IsPrecompiled() const;

		EngineContext* GetEngineContext();

	private:


		void ReadShaderSource();
		NVRHI::ShaderType::Enum GetShaderTypeByName(const String& name);
		String GetFeatureLevelForShaderType(const NVRHI::ShaderType::Enum& type);
	};
}