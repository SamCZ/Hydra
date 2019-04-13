#pragma once

#include "Hydra/Core/Library.h"
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

		//InputEventAction(String actionName, InputEvent eventType, SharedPtr<IDelegate<ReturnType, ArgsType...>> func) : ActionName(actionName), EventType(eventType), Delegate(func) {}
	};

	class InputManager;
	DEFINE_PTR(InputManager)

	class HYDRA_API InputManager
	{
	protected:
		List<InputActionKeyMapping> _ActionMappings;
		List<InputAxisKeyMapping> _AxisMappings;

		List<InputEventAction<void>> _InputActionsListeners;
		List<InputEventAction<void, char>> _InputTypeListeners;

		List<InputEventAction<void, float>> _InputAxisListeners;

		Vector2i _MousePos = Vector2i(-1, -1);
		Vector2i _LastMousePos = Vector2i(-1, -1);

		Map<Key, bool> _KeyStates;

		bool _MouseCaptured;
		bool _MouseHiddenState;
		bool _MouseShowState;
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

		template<class UserClass>
		inline void BindKeyTypeAction(UserClass* object, typename Delegate<UserClass, void, char>::MethodPtr fnc)
		{
			Delegate<UserClass, void, char>* del = new Delegate<UserClass, void, char>(object, fnc);
			SharedPtr<IDelegate<void, char>> baseDelegate = MakeShareable(del);

			InputEventAction<void, char> action;
			action.Delegate = baseDelegate;

			_InputTypeListeners.push_back(action);
		}
		
		template<class UserClass>
		inline void BindAxis(String actionName, UserClass* object, typename Delegate<UserClass, void, float>::MethodPtr fnc)
		{
			Delegate<UserClass, void, float>* del = new Delegate<UserClass, void, float>(object, fnc);
			SharedPtr<IDelegate<void, float>> baseDelegate = MakeShareable(del);

			InputEventAction<void, float> action = {
				actionName, InputEvent::IE_Axis, baseDelegate
			};

			_InputAxisListeners.push_back(action);
		}

		void SetMouseCapture(bool state);
		void ToggleMouseCapture();
		bool IsMouseCaptured() const;

		virtual void Update();

		bool OnKeyChar(const char Character, const bool IsRepeat);
		bool OnKeyDown(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat);
		bool OnKeyUp(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat);

		Key MouseButtonToKey(const EMouseButtons::Type btn);

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

		Vector2i GetCursorPos() const;
	;};
}