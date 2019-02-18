#include "Hydra/Core/File.h"

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <stack>

#ifdef USE_BOOST
#include "boost/filesystem.hpp"
#endif

#include <algorithm>

namespace Hydra
{
	File::~File()
	{

	}

	File::File() : File("Temp")
	{

	}

	File::File(const char* file) : File(String(file))
	{

	}

	File::File(String file)
	{
		_rootPath = FixPath(file);
	}

	File::File(File parent, String file) : File(parent)
	{
		String fixedFile = FixPath(file);
		if (_rootPath[_rootPath.length() - 1] != '/')
		{
			_rootPath += '/';
		}
		_rootPath += fixedFile;
	}

	String File::FixPath(String path, const char from, const char to)
	{
		for (int i = 0; i < path.length(); i++)
		{
			if (path[i] == from)
			{
				path[i] = to;
			}
		}
		return path;
	}

	String File::GetPath() const
	{
		return _rootPath;
	}

	String File::GetParent() const
	{
		return _rootPath.substr(0, _rootPath.find_last_of('/'));
	}

	File File::GetParentFile() const
	{
		return File(GetParent());
	}

	bool File::IsExist() const
	{
		struct stat buffer;
		return (stat(_rootPath.c_str(), &buffer) == 0);
	}

	bool File::Mkdirs() const
	{
#ifdef USE_BOOST
		return boost::filesystem::create_directories(getPath());
#endif
		return false;
	}

#undef DeleteFile

	bool File::DeleteFile() const
	{
		if (GetExtension() != "meta")
		{
			File metaFile(GetPath() + ".meta");
			if (metaFile.IsExist())
			{
				metaFile.DeleteFile();
			}
		}
#ifdef USE_BOOST
		if (boost::filesystem::exists(getPath()))
			return boost::filesystem::remove(getPath());
#endif
		return false;
	}

#define DeleteFileW DeleteFile

	bool File::IsDirectory() const
	{
		struct stat info;
		if (stat(_rootPath.c_str(), &info) != 0)
			return 0;
		else if (info.st_mode & S_IFDIR)
			return 1;
		else
			return 0;
	}

	String File::GetName() const
	{
		String path = _rootPath;
		if (_rootPath.find_last_of('/') == _rootPath.length() - 1)
		{
			path = _rootPath.substr(0, _rootPath.find_last_of('/'));
		}
		return _rootPath.substr(_rootPath.find_last_of('/') + 1);
	}

	String File::GetCleanName() const
	{
		String name = GetName();
		size_t dotIndex = name.find_last_of('.');
		if (dotIndex > 0)
		{
			return name.substr(0, dotIndex);
		}
		else
		{
			return name;
		}
	}

	String File::GetExtension(bool lower) const
	{
		String name = GetName();
		String ext = name.substr(name.find_last_of('.') + 1);
		if (lower)
		{
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		}
		return ext;
	}

	bool File::HaveExtension(String ext)
	{
		return GetExtension() == ext;
	}

	List<File> File::ListFiles(bool recursive)
	{
		List<File> files;
		if (!IsDirectory()) return files;
#ifdef USE_BOOST
		namespace fs = boost::filesystem;

		fs::path apk_path(getPath());
		if (recursive)
		{
			fs::recursive_directory_iterator end;

			for (fs::recursive_directory_iterator i(apk_path); i != end; ++i)
			{
				const fs::path cp = (*i);
				files.push_back(File(cp.string()));
			}
		}
		else
		{
			fs::directory_iterator end;

			for (fs::directory_iterator i(apk_path); i != end; ++i)
			{
				const fs::path cp = (*i);
				files.push_back(File(cp.string()));
			}
		}
#endif
#if true
		HANDLE hFind = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAA ffd;
		String path;
		String spec;
		std::stack<String> directories;

		directories.push(GetPath());

		while (!directories.empty())
		{
			path = directories.top();
			spec = path + "\\";// +mask;
			directories.pop();

			hFind = FindFirstFileA(spec.c_str(), &ffd);
			if (hFind == INVALID_HANDLE_VALUE)
			{
				continue;
			}

			do
			{
				if (strcmp(ffd.cFileName, ".") != 0 &&
					strcmp(ffd.cFileName, "..") != 0)
				{
					if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						directories.push(path + "\\" + ffd.cFileName);
						files.push_back(path + "/" + ffd.cFileName);
					}
					else
					{
						files.push_back(path + "/" + ffd.cFileName);
					}
				}
			} while (FindNextFileA(hFind, &ffd) != 0);

			if (GetLastError() != ERROR_NO_MORE_FILES)
			{
				FindClose(hFind);
				continue;
			}

			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE;
		}
#endif

		return files;
	}


	std::ostream& operator<<(std::ostream& os, const File& obj)
	{
		os << obj.GetPath();
		return os;
	}

	bool operator==(const File& left, const File& right)
	{
		return left.GetPath() == right.GetPath();
	}

	bool operator!=(const File& left, const File& right)
	{
		return !(left == right);
	}
}