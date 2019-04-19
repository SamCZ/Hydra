#pragma once

#include "Hydra/Core/File.h"
#include "IStream.h"

class FileStream : public IFileStream
{
private:
	File _File;
public:
	FileStream(const File& file);

	Blob* Read();
};