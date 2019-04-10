#pragma once

#include <Windows.h>

#include "Hydra/Input/InputManager.h"

namespace Hydra
{
	class EngineContext;

	struct EModifierKey
	{
		enum Type
		{
			LeftShift,		// VK_LSHIFT
			RightShift,		// VK_RSHIFT
			LeftControl,	// VK_LCONTROL
			RightControl,	// VK_RCONTROL
			LeftAlt,		// VK_LMENU
			RightAlt,		// VK_RMENU
			CapsLock,		// VK_CAPITAL
			Count,
		};
	};

	class WindowsInputManager : public InputManager
	{
	private:
		EngineContext* _Context;

		bool ModifierKeyState[EModifierKey::Count];
	public:
		WindowsInputManager(EngineContext* context);

		LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		bool IsInputMessage(uint32 msg);
		bool IsKeyboardInputMessage(uint32 msg);
		bool IsMouseInputMessage(uint32 msg);

		void UpdateAllModifierKeyStates();

	public:
		virtual void Update() override;

	};

	DEFINE_PTR(WindowsInputManager)
}