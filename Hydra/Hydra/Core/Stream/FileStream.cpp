#include "FileStream.h"

FileStream::FileStream(const File & file) : _File(file)
{
}

Blob* FileStream::Read()
{
	std::streampos size;
	char* memblock;

	std::ifstream file(_File.GetPath(), std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open())
	{
		size = file.tellg();
		memblock = new char[size];
		file.seekg(0, std::ios::beg);
		file.read(memblock, size);
		file.close();

		return new Blob(memblock, size);
	}

	return nullptr;
}
