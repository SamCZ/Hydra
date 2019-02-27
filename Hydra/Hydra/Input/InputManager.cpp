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
	bool InputManager::OnKeyChar(const char Character, const bool IsRepeat)
	{
		return false;
	}
	bool InputManager::OnKeyDown(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat)
	{
		if (IsRepeat) return false; // this disables modifier keys

		Key key = InputKeyManager::Get().GetKeyFromCodes(KeyCode, CharacterCode);

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

	bool InputManager::OnMouseDown(const EMouseButtons::Type Button, const Vector2i CursorPos)
	{
		

		return false;
	}

	bool InputManager::OnMouseUp(const EMouseButtons::Type Button, const Vector2i CursorPos)
	{
		return false;
	}

	bool InputManager::OnMouseDoubleClick(const EMouseButtons::Type Button, const Vector2i CursorPos)
	{
		return false;
	}

	bool InputManager::OnMouseWheel(const float Delta, const Vector2i CursorPos)
	{

		return false;
	}

	bool InputManager::OnMouseMove()
	{
		return false;
	}

	bool InputManager::OnRawMouseMove(const Vector2i CursorPos)
	{
		std::cout << CursorPos.x << ":" << CursorPos.y << std::endl;
		return false;
	}
}