#pragma once

#include "Hydra/Render/Shader.h"

namespace Hydra
{
	class File;

	class ShaderImporter
	{
	public:
		static ShaderPtr Import(const File& file);
	};
}