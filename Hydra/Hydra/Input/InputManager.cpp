#include "Hydra/Input/InputManager.h"

namespace Hydra
{
	void InputManager::AddActionMapping(const InputActionKeyMapping & mapping)
	{
		_ActionMappings.push_back(mapping);
	}

	void InputManager::AddActionMapping(const String InActionName, const Key InKey, const bool bInShift, const bool bInCtrl, const bool bInAlt, const bool bInCmd)
	{
		InputActionKeyMapping mapping = {
			InActionName, InKey, bInShift, bInCtrl, bInAlt, bInCmd
		};

		_ActionMappings.push_back(mapping);
	}

	void InputManager::RemoveActionMapping(const InputActionKeyMapping & mapping)
	{
		List_Remove(_ActionMappings, mapping);
	}

	void InputManager::AddAxisMapping(const InputAxisKeyMapping & mapping)
	{
		_AxisMappings.push_back(mapping);
	}

	void InputManager::AddAxisMapping(const String InAxisName, const Key InKey, const float InScale)
	{
		InputAxisKeyMapping mapping = { InAxisName, InKey, InScale };
		_AxisMappings.push_back(mapping);
	}

	void InputManager::RemoveAxisMapping(const InputAxisKeyMapping & mapping)
	{
		List_Remove(_AxisMappings, mapping);
	}

	void InputManager::ReadInputMapping(const File & file)
	{
	}

	void InputManager::Update()
	{
		ITER(_KeyStates, it)
		{
			if (it->second)
			{
				const Key& key = it->first;

				for (InputAxisKeyMapping& mapping : _AxisMappings)
				{
					if (mapping.KeyType != key)
					{
						continue;
					}

					for (InputEventAction<void, float>& action : _InputAxisListeners)
					{
						if (mapping.AxisName == action.ActionName)
						{
							action.Delegate->Invoke(1.0f * mapping.Scale);
						}
					}
				}
			}
		}

		if (_MousePos != _LastMousePos)
		{
			Vector2i mouseDelta = _MousePos - _LastMousePos;

			for (InputAxisKeyMapping& mapping : _AxisMappings)
			{
				for (InputEventAction<void, float>& action : _InputAxisListeners)
				{
					if (mapping.AxisName == action.ActionName)
					{
						if (mapping.KeyType == Keys::MouseX)
						{
							action.Delegate->Invoke(mouseDelta.x * mapping.Scale);
						}

						if (mapping.KeyType == Keys::MouseY)
						{
							action.Delegate->Invoke(mouseDelta.y * mapping.Scale);
						}
					}
				}
			}

			_LastMousePos = _MousePos;
		}
	}

	void Hydra::InputManager::SetMouseCapture(bool state)
	{
		_MouseCaptured = state;
	}

	void Hydra::InputManager::ToggleMouseCapture()
	{
		_MouseCaptured = !_MouseCaptured;
	}

	bool Hydra::InputManager::IsMouseCaptured() const
	{
		return _MouseCaptured;
	}

	bool InputManager::OnKeyChar(const char Character, const bool IsRepeat)
	{
		for (InputEventAction<void, char>& action : _InputTypeListeners)
		{
			action.Delegate->Invoke(Character);
		}

		return false;
	}

	bool InputManager::OnKeyDown(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat)
	{
		//if (IsRepeat) return false; // this disables modifier keys

		Key key = InputKeyManager::Get().GetKeyFromCodes(KeyCode, CharacterCode);

		_KeyStates[key] = true;

		for (InputActionKeyMapping& mapping : _ActionMappings)
		{
			if (mapping.KeyType == key)
			{
				//Call delegates
				for (InputEventAction<void>& action : _InputActionsListeners)
				{
					if (action.ActionName == mapping.ActionName && action.EventType == IE_Pressed)
					{
						action.Delegate->Invoke();
					}
				}
			}
		}

		return false;
	}

	bool InputManager::OnKeyUp(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat)
	{
		Key key = InputKeyManager::Get().GetKeyFromCodes(KeyCode, CharacterCode);

		_KeyStates[key] = false;

		for (InputActionKeyMapping& mapping : _ActionMappings)
		{
			if (mapping.KeyType == key)
			{
				//Call delegates
				for (InputEventAction<void>& action : _InputActionsListeners)
				{
					if (action.ActionName == mapping.ActionName && action.EventType == IE_Released)
					{
						action.Delegate->Invoke();
					}
				}
			}
		}

		return false;
	}

	Key InputManager::MouseButtonToKey(const EMouseButtons::Type btn)
	{
		switch (btn)
		{
		case EMouseButtons::Left:
			return Keys::LeftMouseButton;
		case EMouseButtons::Right:
			return Keys::RightMouseButton;
		case EMouseButtons::Thumb01:
			return Keys::ThumbMouseButton;
		case EMouseButtons::Thumb02:
			return Keys::ThumbMouseButton2;

		default:
			return Keys::Invalid;
		}
	}

	bool InputManager::OnMouseDown(const EMouseButtons::Type Button, const Vector2i CursorPos)
	{
		Key key = MouseButtonToKey(Button);

		for (InputActionKeyMapping& mapping : _ActionMappings)
		{
			if (mapping.KeyType == key)
			{
				for (InputEventAction<void>& action : _InputActionsListeners)
				{
					if (action.ActionName == mapping.ActionName && action.EventType == IE_Pressed)
					{
						action.Delegate->Invoke();
					}
				}
			}
		}

		return false;
	}

	bool InputManager::OnMouseUp(const EMouseButtons::Type Button, const Vector2i CursorPos)
	{
		Key key = MouseButtonToKey(Button);

		for (InputActionKeyMapping& mapping : _ActionMappings)
		{
			if (mapping.KeyType == key)
			{
				for (InputEventAction<void>& action : _InputActionsListeners)
				{
					if (action.ActionName == mapping.ActionName && action.EventType == IE_Released)
					{
						action.Delegate->Invoke();
					}
				}
			}
		}

		return false;
	}

	bool InputManager::OnMouseDoubleClick(const EMouseButtons::Type Button, const Vector2i CursorPos)
	{
		//TODO: Not working !
		Key key = MouseButtonToKey(Button);

		for (InputActionKeyMapping& mapping : _ActionMappings)
		{
			if (mapping.KeyType == key)
			{
				for (InputEventAction<void>& action : _InputActionsListeners)
				{
					if (action.ActionName == mapping.ActionName && action.EventType == IE_DoubleClick)
					{
						action.Delegate->Invoke();
					}
				}
			}
		}

		return false;
	}

	bool InputManager::OnMouseWheel(const float Delta, const Vector2i CursorPos)
	{
		for (InputAxisKeyMapping& mapping : _AxisMappings)
		{
			if (mapping.KeyType != Keys::MouseWheelAxis)
			{
				continue;
			}

			for (InputEventAction<void, float>& action : _InputAxisListeners)
			{
				if (mapping.AxisName == action.ActionName)
				{
					action.Delegate->Invoke(Delta * mapping.Scale);
				}
			}
		}

		Key key;
		if (Delta > 0)
		{
			key = Keys::MouseScrollUp;
		}
		else
		{
			key = Keys::MouseScrollDown;
		}

		for (InputActionKeyMapping& mapping : _ActionMappings)
		{
			if (mapping.KeyType == key)
			{
				for (InputEventAction<void>& action : _InputActionsListeners)
				{
					if (action.ActionName == mapping.ActionName)
					{
						action.Delegate->Invoke();
					}
				}
			}
		}

		return false;
	}

	bool InputManager::OnMouseMove()
	{
		return false;
	}

	bool InputManager::OnRawMouseMove(const Vector2i CursorPos)
	{
		_MousePos = CursorPos;

		return false;
	}
	Vector2i InputManager::GetCursorPos() const
	{
		return _LastMousePos;
	}
}