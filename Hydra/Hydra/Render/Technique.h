#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Core/Resource.h"
#include "Hydra/Core/File.h"
#include "Hydra/Core/Container.h"

#include "Hydra/Render/Shader.h"

namespace Hydra
{
	class Technique : Resource
	{
	private:
		File _Source;
		String _RenderStage;

		FastMap<NVRHI::ShaderType::Enum, Shader*> _Shaders;
	public:
		Technique(const File& source);
		~Technique();

		void Recompile(bool loadNewSource = true);

		
		ShaderPtr GetShader(const NVRHI::ShaderType::Enum& type);
		NVRHI::ShaderHandle GetRawShader(const NVRHI::ShaderType::Enum& type);
		ID3DBlob* GetShaderBlob(const NVRHI::ShaderType::Enum& type);

	private:
		String GetFeatureLevelForShaderType(const NVRHI::ShaderType::Enum& type);
	};

	DEFINE_PTR(Technique)

#define _TECH(TechName) MakeShared<Technique>(TechName)
}