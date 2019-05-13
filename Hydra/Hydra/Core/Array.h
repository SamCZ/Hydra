#pragma once

#include "Hydra/Core/Library.h"
#include <vector>

template <typename T>
class Array
{
private:
	std::vector<T> _Data;
public:
	inline Array(int size)
	{
		_Data.reserve(size);
	}

	inline T operator[](const int index)
	{
		return _Data[index];
	}
};