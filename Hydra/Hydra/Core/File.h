#pragma once

#include <string>
#include <vector>

#include "Hydra/Core/Library.h"
#include "Hydra/Core/String.h"
#include "Hydra/Core/Container.h"

class HYDRA_API File
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

	File GetPathAt(const String& folder) const;

	bool IsExist() const;
	bool IsDirectory() const;
	bool Mkdirs() const;
	bool DeleteFile() const;

	String GetName() const;
	String GetCleanName() const;
	String GetExtension(bool lower = true) const;
	bool HaveExtension(String ext);

	List<File> ListFiles(bool recursive = false);

	List<String> ReadLines() const;

	static String FixPath(String path, const char from = '\\', const char to = '/');

	operator String() const
	{
		return GetPath();
	}

private:
	String _rootPath;
};

HYDRA_API std::ostream& operator<<(std::ostream& os, const File& obj);
HYDRA_API bool operator==(const File& left, const File& right);
HYDRA_API bool operator!=(const File& left, const File& right);