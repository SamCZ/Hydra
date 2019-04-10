#pragma once

#include "Hydra/Input/GenericPlatformInput.h"

namespace Hydra
{
	class HYDRA_EXPORT WindowsPlatformInput : public GenericPlatformInput
	{
	public:
		static uint32 GetKeyMap(uint32* KeyCodes, String* KeyNames, uint32 MaxMappings);
		static uint32 GetCharKeyMap(uint32* KeyCodes, String* KeyNames, uint32 MaxMappings);
	};

	typedef WindowsPlatformInput PlatformInput;
}