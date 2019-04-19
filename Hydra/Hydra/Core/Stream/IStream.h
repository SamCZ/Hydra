#pragma once

#include <iostream>
#include <fstream>

#include "Blob.h"

class IFileStream
{
public:
	virtual Blob* Read() = 0;
};