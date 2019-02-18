#pragma once

#include <string>
#include <vector>

#include "Hydra/Core/String.h"
#include "Hydra/Core/Container.h"

namespace Hydra
{
	class File
	{
	public:
		~File();
		File();
		File(const char* file);
		File(String file);
		File(File parent, String file);

		String GetPath() const;
		String GetParent() const;
		File GetParentFile() const;

		bool IsExist() const;
		bool IsDirectory() const;
		bool Mkdirs() const;
		bool DeleteFile() const;

		String GetName() const;
		String GetCleanName() const;
		String GetExtension(bool lower = true) const;
		bool HaveExtension(String ext);

		List<File> ListFiles(bool recursive = false);

		static String FixPath(String path, const char from = '\\', const char to = '/');
	private:
		String _rootPath;
	};

	std::ostream& operator<<(std::ostream& os, const File& obj);
	bool operator==(const File& left, const File& right);
	bool operator!=(const File& left, const File& right);
}