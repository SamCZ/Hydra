#pragma once

#include "Hydra/Core/EObject.h"
#include "Hydra/Core/Common.h"
#include "Hydra/Core/Delegate.h"

#include "Hydra/Input/InputCoreTypes.h"

namespace Hydra
{
	enum InputEvent
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
		List<IDelegate<void>*> _InputActions;
		List<IDelegate<void, float>*> _InputAxesActions;

	public:

		template<class UserClass>
		inline void BindAction(String actionName, const InputEvent& keyEvent, UserClass* object, typename Delegate<UserClass, void>::MethodPtr fnc)
		{
			Delegate<UserClass, void>* del = new Delegate<UserClass, void>(object, fnc);
			_InputActions.push_back(del);
		}

		void CallActions()
		{
			for (IDelegate<void>* del : _InputActions)
			{
				del->Invoke();
			}
		}
		//void BindAxis();
	;};
}