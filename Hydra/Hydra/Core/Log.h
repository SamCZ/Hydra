#pragma once

#include <iostream>
#include "Hydra/Core/String.h"
#include <assert.h>

namespace Hydra
{
	inline static void Log(const String& message)
	{
		std::cout << message << std::endl;
	}

	inline static void Log(const String& funcName, const String& message)
	{
		std::cout << funcName << ": " << message << std::endl;
	}

	inline static void Log(const String& funcName, const String& argument, const String& message)
	{
		std::cout << funcName << "(" << argument << "): " << message << std::endl;
	}

	inline static void LogError(const String& funcName, const String& message)
	{
		std::cerr << funcName << ": " << message << std::endl;
	}

	inline static void LogError(const String& funcName, const String& argument, const String& message)
	{
		std::cerr << funcName << "(" << argument << "): " << message << std::endl;
	}
}