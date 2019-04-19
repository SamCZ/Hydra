#pragma once

#include "Hydra/Core/Common.h"

struct WinTiming
{
	HYDRA_API static double getTime();
	HYDRA_API static void sleep(uint32 milliseconds);
};

typedef WinTiming PlatformTiming;

namespace Time
{
	inline double getTime()
	{
		return PlatformTiming::getTime();
	}

	inline void sleep(uint32 milliseconds)
	{
		PlatformTiming::sleep(milliseconds);
	}
};