#pragma once

#include <iostream>
#include "Hydra/Core/String.h"

namespace Hydra
{
	inline static void Log(const String& funcName, const String& message)
	{
		std::cout << funcName << ": " << message << std::endl;
	}
}