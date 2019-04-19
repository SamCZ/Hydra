#pragma once

class Blob
{
private:
	char* _Data;
	size_t _Size;
public:
	Blob(char* data, size_t size) : _Data(data), _Size(size)
	{

	}

	~Blob()
	{
		delete[] _Data;
	}

	size_t GetDataSize()
	{
		return _Size;
	}

	char* GetData()
	{
		return _Data;
	}
};