#pragma once

#include "Hydra/Core/Common.h"

namespace Hydra
{
	struct WinTiming
	{
		static double getTime();
		static void sleep(uint32 milliseconds);
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
}