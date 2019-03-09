#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Core/Resource.h"
#include "Hydra/Core/File.h"
#include "Hydra/Core/Container.h"

#include "Hydra/Render/Shader.h"

namespace Hydra
{
	struct ShaderSource
	{
		String Source;

		Map<NVRHI::ShaderType::Enum, String> ShaderTypes;
		Map<NVRHI::ShaderType::Enum, List<String>> Keywords;
		Map<String, int> KeywordIndexMap;
	};

	class Technique : Resource
	{
	private:
		File _Source;
		String _RenderStage;
		ShaderSource* _ShaderSource;

		FastMap<NVRHI::ShaderType::Enum, Shader*> _Shaders;

		uint32 _TestHash = 0;
	public:
		Technique(const File& source);
		~Technique();

		void Recompile(bool loadNewSource = false);

		void SetKeywordByHash(uint32 hash);
		uint32 GetKeywordHash(const List<String>& keywords);
		
		ShaderPtr GetShader(const NVRHI::ShaderType::Enum& type);
		NVRHI::ShaderHandle GetRawShader(const NVRHI::ShaderType::Enum& type);
		ID3DBlob* GetShaderBlob(const NVRHI::ShaderType::Enum& type);

	private:

		ShaderSource* CreateShaderSource();

		NVRHI::ShaderType::Enum GetShaderTypeByName(const String& name);
		String GetFeatureLevelForShaderType(const NVRHI::ShaderType::Enum& type);
	};

	DEFINE_PTR(Technique)

#define _TECH(TechName) MakeShared<Technique>(TechName)
}