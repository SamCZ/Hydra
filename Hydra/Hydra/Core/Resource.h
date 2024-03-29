#pragma once

#include "Hydra/Core/Library.h"
#include "Hydra/Core/String.h"

class HYDRA_API Resource
{
protected:
	String _Source;
public:

	void SetSource(const String& source);
	const String GetSource();
};