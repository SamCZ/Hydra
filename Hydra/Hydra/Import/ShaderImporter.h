#pragma once

namespace Hydra
{
	class Shader;
	class File;

	class ShaderImporter
	{
	public:
		static Shader* Import(const File& file);
	};
}