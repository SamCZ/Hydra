#pragma once

#include "Hydra/Core/Common.h"

#include "Hydra/Input/InputCoreTypes.h"

namespace Hydra
{
	enum EInputEvent
	{
		IE_Pressed = 0,
		IE_Released = 1,
		IE_Repeat = 2,
		IE_DoubleClick = 3,
		IE_Axis = 4,
		IE_MAX = 5,
	};


	class InputManager;
	DEFINE_PTR(InputManager)

	class InputManager
	{
	private:


	public:

	};
}