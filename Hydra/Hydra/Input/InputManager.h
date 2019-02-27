#pragma once

#include "Hydra/Core/EObject.h"
#include "Hydra/Core/Common.h"
#include "Hydra/Core/Delegate.h"
#include "Hydra/Core/File.h"
#include "Hydra/Core/Vector.h"

#include "Hydra/Input/InputMapping.h"

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

	namespace EMouseButtons
	{
		enum Type
		{
			Left = 0,
			Middle,
			Right,
			Thumb01,
			Thumb02,

			Invalid,
		};
	}

	template<typename ReturnType, typename... ArgsType>
	struct InputEventAction
	{
		String ActionName;
		InputEvent EventType;
		SharedPtr<IDelegate<ReturnType, ArgsType...>> Delegate;

		InputEventAction(String actionName, InputEvent eventType, SharedPtr<IDelegate<ReturnType, ArgsType...>> func) : ActionName(actionName), EventType(eventType), Delegate(func) {}
	};

	class InputManager;
	DEFINE_PTR(InputManager)

	class InputManager
	{
	private:
		List<InputActionKeyMapping> _ActionMappings;
		List<InputAxisKeyMapping> _AxisMappings;

		List<InputEventAction<void>> _InputActionsListeners;

	public:
		void AddActionMapping(const InputActionKeyMapping& mapping);
		void AddActionMapping(const String InActionName, const Key InKey, const bool bInShift = false, const bool bInCtrl = false, const bool bInAlt = false, const bool bInCmd = false);
		void RemoveActionMapping(const InputActionKeyMapping& mapping);

		void AddAxisMapping(const InputAxisKeyMapping& mapping);
		void AddAxisMapping(const String InAxisName, const Key InKey, const float InScale = 1.f);
		void RemoveAxisMapping(const InputAxisKeyMapping& mapping);

		void ReadInputMapping(const File& file);

		template<class UserClass>
		inline void BindAction(String actionName, const InputEvent& keyEvent, UserClass* object, typename Delegate<UserClass, void>::MethodPtr fnc)
		{
			Delegate<UserClass, void>* del = new Delegate<UserClass, void>(object, fnc);
			SharedPtr<IDelegate<void>> baseDelegate = MakeShareable(del);

			InputEventAction<void> action = {
				actionName, keyEvent, baseDelegate
			};

			_InputActionsListeners.push_back(action);
		}
		//void BindAxis();



		bool OnKeyChar(const char Character, const bool IsRepeat);
		bool OnKeyDown(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat);
		bool OnKeyUp(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat);

		//bool OnMouseDown(const EMouseButtons::Type Button);
		bool OnMouseDown(const EMouseButtons::Type Button, const Vector2i CursorPos);
		//bool OnMouseUp(const EMouseButtons::Type Button);
		bool OnMouseUp(const EMouseButtons::Type Button, const Vector2i CursorPos);
		//bool OnMouseDoubleClick( const EMouseButtons::Type Button);
		bool OnMouseDoubleClick(const EMouseButtons::Type Button, const Vector2i CursorPos);
		//bool OnMouseWheel(const float Delta);
		bool OnMouseWheel(const float Delta, const Vector2i CursorPos);
		bool OnMouseMove();
		bool OnRawMouseMove(const Vector2i CursorPos);
	;};
}