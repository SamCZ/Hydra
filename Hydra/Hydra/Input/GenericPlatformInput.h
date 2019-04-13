#pragma once

#include "Hydra/Core/Common.h"

namespace Hydra
{
	class HYDRA_API GenericPlatformInput
	{
	public:
		FORCEINLINE static uint32 GetKeyMap(uint32* KeyCodes, String* KeyNames, uint32 MaxMappings)
		{
			return 0;
		}

		FORCEINLINE static uint32 GetCharKeyMap(uint32* KeyCodes, String* KeyNames, uint32 MaxMappings)
		{
			return 0;
		}

	protected:
		static uint32 GetStandardPrintableKeyMap(uint32* KeyCodes, String* KeyNames, uint32 MaxMappings, bool bMapUppercaseKeys, bool bMapLowercaseKeys);
	};
}