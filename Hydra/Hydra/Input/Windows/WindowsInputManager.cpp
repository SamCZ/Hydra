#include "Hydra/Input/Windows/WindowsInputManager.h"

#include <iostream>
#include <windowsx.h>

#include "Hydra/Core/Vector.h"
#include "Hydra/EngineContext.h"

#include "Hydra/Render/Pipeline/Windows/DX11/DeviceManager11.h"

namespace Hydra
{
	WindowsInputManager::WindowsInputManager(EngineContext * context) : _Context(context)
	{
	}
	LRESULT WindowsInputManager::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (IsInputMessage(msg))
		{
			if (IsKeyboardInputMessage(msg))
			{
				// Force an update since we may have just consumed a modifier key state change
				UpdateAllModifierKeyStates();
			}
			//return S_OK;	// consume input messages
		}


		switch (msg)
		{
		case WM_INPUTLANGCHANGEREQUEST:
		case WM_INPUTLANGCHANGE:
		case WM_IME_SETCONTEXT:
		case WM_IME_NOTIFY:
		case WM_IME_REQUEST:
		case WM_IME_STARTCOMPOSITION:
		case WM_IME_COMPOSITION:
		case WM_IME_ENDCOMPOSITION:
		case WM_IME_CHAR:
		{
			/*if (TextInputMethodSystem.IsValid())
			{
				TextInputMethodSystem->ProcessMessage(hwnd, msg, wParam, lParam);
			}*/
			return 0;
		}
		break;
		// Character
		case WM_CHAR:
		{
			// Character code is stored in WPARAM
			const TCHAR Character = static_cast<const TCHAR>(wParam);

			// LPARAM bit 30 will be ZERO for new presses, or ONE if this is a repeat
			const bool bIsRepeat = (lParam & 0x40000000) != 0;

			char Char;
			WideCharToMultiByte(CP_UTF8, 0, &Character, 1, &Char, 1, NULL, NULL);

			OnKeyChar(Char, bIsRepeat);

			// Note: always return 0 to handle the message.  Win32 beeps if WM_CHAR is not handled...
			return 0;
		}
		break;


		// Key down
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			// Character code is stored in WPARAM
			const int32 Win32Key = static_cast<const int32>(wParam);

			// The actual key to use.  Some keys will be translated into other keys. 
			// I.E VK_CONTROL will be translated to either VK_LCONTROL or VK_RCONTROL as these
			// keys are never sent on their own
			int32 ActualKey = Win32Key;

			// LPARAM bit 30 will be ZERO for new presses, or ONE if this is a repeat
			bool bIsRepeat = (lParam & 0x40000000) != 0;

			switch (Win32Key)
			{
			case VK_MENU:
				// Differentiate between left and right alt
				if ((lParam & 0x1000000) == 0)
				{
					ActualKey = VK_LMENU;
					bIsRepeat = ModifierKeyState[EModifierKey::LeftAlt];
					ModifierKeyState[EModifierKey::LeftAlt] = true;
				}
				else
				{
					ActualKey = VK_RMENU;
					bIsRepeat = ModifierKeyState[EModifierKey::RightAlt];
					ModifierKeyState[EModifierKey::RightAlt] = true;
				}
				break;
			case VK_CONTROL:
				// Differentiate between left and right control
				if ((lParam & 0x1000000) == 0)
				{
					ActualKey = VK_LCONTROL;
					bIsRepeat = ModifierKeyState[EModifierKey::LeftControl];
					ModifierKeyState[EModifierKey::LeftControl] = true;
				}
				else
				{
					ActualKey = VK_RCONTROL;
					bIsRepeat = ModifierKeyState[EModifierKey::RightControl];
					ModifierKeyState[EModifierKey::RightControl] = true;
				}
				break;
			case VK_SHIFT:
				// Differentiate between left and right shift
				ActualKey = MapVirtualKey((lParam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
				if (ActualKey == VK_LSHIFT)
				{
					bIsRepeat = ModifierKeyState[EModifierKey::LeftShift];
					ModifierKeyState[EModifierKey::LeftShift] = true;
				}
				else
				{
					bIsRepeat = ModifierKeyState[EModifierKey::RightShift];
					ModifierKeyState[EModifierKey::RightShift] = true;
				}
				break;
			case VK_CAPITAL:
				ModifierKeyState[EModifierKey::CapsLock] = (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
				break;
			default:
				// No translation needed
				break;
			}

			// Get the character code from the virtual key pressed.  If 0, no translation from virtual key to character exists
			uint32 CharCode = ::MapVirtualKey(Win32Key, MAPVK_VK_TO_CHAR);

			const bool Result = OnKeyDown(ActualKey, CharCode, bIsRepeat);

			// Always return 0 to handle the message or else windows will beep
			if (Result || msg != WM_SYSKEYDOWN)
			{
				// Handled
				return 0;
			}
		}
		break;


		// Key up
		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			// Character code is stored in WPARAM
			int32 Win32Key = static_cast<int32>(wParam);

			// The actual key to use.  Some keys will be translated into other keys. 
			// I.E VK_CONTROL will be translated to either VK_LCONTROL or VK_RCONTROL as these
			// keys are never sent on their own
			int32 ActualKey = Win32Key;

			bool bModifierKeyReleased = false;
			switch (Win32Key)
			{
			case VK_MENU:
				// Differentiate between left and right alt
				if ((lParam & 0x1000000) == 0)
				{
					ActualKey = VK_LMENU;
					ModifierKeyState[EModifierKey::LeftAlt] = false;
				}
				else
				{
					ActualKey = VK_RMENU;
					ModifierKeyState[EModifierKey::RightAlt] = false;
				}
				break;
			case VK_CONTROL:
				// Differentiate between left and right control
				if ((lParam & 0x1000000) == 0)
				{
					ActualKey = VK_LCONTROL;
					ModifierKeyState[EModifierKey::LeftControl] = false;
				}
				else
				{
					ActualKey = VK_RCONTROL;
					ModifierKeyState[EModifierKey::RightControl] = false;
				}
				break;
			case VK_SHIFT:
				// Differentiate between left and right shift
				ActualKey = MapVirtualKey((lParam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
				if (ActualKey == VK_LSHIFT)
				{
					ModifierKeyState[EModifierKey::LeftShift] = false;
				}
				else
				{
					ModifierKeyState[EModifierKey::RightShift] = false;
				}
				break;
			case VK_CAPITAL:
				ModifierKeyState[EModifierKey::CapsLock] = (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
				break;
			default:
				// No translation needed
				break;
			}

			// Get the character code from the virtual key pressed.  If 0, no translation from virtual key to character exists
			uint32 CharCode = ::MapVirtualKey(Win32Key, MAPVK_VK_TO_CHAR);

			// Key up events are never repeats
			const bool bIsRepeat = false;

			const bool Result = OnKeyUp(ActualKey, CharCode, bIsRepeat);

			// Note that we allow system keys to pass through to DefWndProc here, so that core features
			// like Alt+F4 to close a window work.
			if (Result || msg != WM_SYSKEYUP)
			{
				// Handled
				return 0;
			}
		}
		break;

		// Mouse Button Down
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_XBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_XBUTTONUP:
		{
			POINT CursorPoint;
			CursorPoint.x = GET_X_LPARAM(lParam);
			CursorPoint.y = GET_Y_LPARAM(lParam);

			//ClientToScreen(hWnd, &CursorPoint);

			const Vector2i CursorPos(CursorPoint.x, CursorPoint.y);

			EMouseButtons::Type MouseButton = EMouseButtons::Invalid;
			bool bDoubleClick = false;
			bool bMouseUp = false;
			switch (msg)
			{
			case WM_LBUTTONDBLCLK:
				bDoubleClick = true;
				MouseButton = EMouseButtons::Left;
				break;
			case WM_LBUTTONUP:
				bMouseUp = true;
				MouseButton = EMouseButtons::Left;
				break;
			case WM_LBUTTONDOWN:
				MouseButton = EMouseButtons::Left;
				break;
			case WM_MBUTTONDBLCLK:
				bDoubleClick = true;
				MouseButton = EMouseButtons::Middle;
				break;
			case WM_MBUTTONUP:
				bMouseUp = true;
				MouseButton = EMouseButtons::Middle;
				break;
			case WM_MBUTTONDOWN:
				MouseButton = EMouseButtons::Middle;
				break;
			case WM_RBUTTONDBLCLK:
				bDoubleClick = true;
				MouseButton = EMouseButtons::Right;
				break;
			case WM_RBUTTONUP:
				bMouseUp = true;
				MouseButton = EMouseButtons::Right;
				break;
			case WM_RBUTTONDOWN:
				MouseButton = EMouseButtons::Right;
				break;
			case WM_XBUTTONDBLCLK:
				bDoubleClick = true;
				MouseButton = (HIWORD(wParam) & XBUTTON1) ? EMouseButtons::Thumb01 : EMouseButtons::Thumb02;
				break;
			case WM_XBUTTONUP:
				bMouseUp = true;
				MouseButton = (HIWORD(wParam) & XBUTTON1) ? EMouseButtons::Thumb01 : EMouseButtons::Thumb02;
				break;
			case WM_XBUTTONDOWN:
				MouseButton = (HIWORD(wParam) & XBUTTON1) ? EMouseButtons::Thumb01 : EMouseButtons::Thumb02;
				break;
			default:
				assert(0);
			}

			if (bMouseUp)
			{
				return OnMouseUp(MouseButton, CursorPos) ? 0 : 1;
			}
			else if (bDoubleClick)
			{
				OnMouseDoubleClick(MouseButton, CursorPos);
			}
			else
			{
				OnMouseDown( MouseButton, CursorPos);
			}
			return 0;
		}
		break;

		// Mouse Movement
		case WM_INPUT:
		{
			std::cout << "move" << std::endl;
			/*POINT CursorPoint;
			CursorPoint.x = GET_X_LPARAM(lParam);
			CursorPoint.y = GET_Y_LPARAM(lParam);

			const Vector2i CursorPos(CursorPoint.x, CursorPoint.y);

			OnRawMouseMove(CursorPos);
			*/
			/*if (DeferredMessage.RawInputFlags == MOUSE_MOVE_RELATIVE)
			{
				MessageHandler->OnRawMouseMove(DeferredMessage.X, DeferredMessage.Y);
			}
			else
			{
				// Absolute coordinates given through raw input are simulated using MouseMove to get relative coordinates
				MessageHandler->OnMouseMove();
			}*/

			return 0;
		}
		break;

		// Mouse Movement
		case WM_NCMOUSEMOVE:
		case WM_MOUSEMOVE:
		{
			POINT CursorPoint;
			CursorPoint.x = GET_X_LPARAM(lParam);
			CursorPoint.y = GET_Y_LPARAM(lParam);

			POINT pos;
			::GetCursorPos(&pos);
			::ScreenToClient(hWnd, &pos);

			CursorPoint.x = pos.x;
			CursorPoint.y = pos.y;

			const Vector2i CursorPos(CursorPoint.x, CursorPoint.y);

			OnRawMouseMove(CursorPos);
		}
		break;
		// Mouse Wheel
		case WM_MOUSEWHEEL:
		{
			const float SpinFactor = 1 / 120.0f;
			const SHORT WheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);

			POINT CursorPoint;
			CursorPoint.x = GET_X_LPARAM(lParam);
			CursorPoint.y = GET_Y_LPARAM(lParam);

			const Vector2i CursorPos(CursorPoint.x, CursorPoint.y);

			const BOOL Result = OnMouseWheel(static_cast<float>(WheelDelta) * SpinFactor, CursorPos);
			return Result ? 0 : 1;
		}
		break;

		// Mouse Cursor
		case WM_SETCURSOR:
		{
			// WM_SETCURSOR - Sent to a window if the mouse causes the cursor to move within a window and mouse input is not captured.
			//TODO: This
			//return MessageHandler->OnCursorSet() ? 0 : 1;
		}
		break;

#if WINVER >= 0x0601
		case WM_TOUCH:
		{
			//TODO: This
			/*UINT InputCount = LOWORD(wParam);
			if (InputCount > 0)
			{
				TUniquePtr<TOUCHINPUT[]> Inputs = MakeUnique<TOUCHINPUT[]>(InputCount);
				if (GetTouchInputInfo((HTOUCHINPUT)lParam, InputCount, Inputs.Get(), sizeof(TOUCHINPUT)))
				{
					for (uint32 i = 0; i < InputCount; i++)
					{
						TOUCHINPUT Input = Inputs[i];
						Vector2 Location(Input.x / 100.0f, Input.y / 100.0f);
						if (Input.dwFlags & TOUCHEVENTF_DOWN)
						{
							int32 TouchIndex = GetTouchIndexForID(Input.dwID);
							if (TouchIndex == INDEX_NONE)
							{
								TouchIndex = GetFirstFreeTouchIndex();
								check(TouchIndex >= 0);

								TouchIDs[TouchIndex] = TOptional<int32>(Input.dwID);
								UE_LOG(LogWindowsDesktop, Verbose, TEXT("OnTouchStarted at (%f, %f), finger %d (system touch id %d)"), Location.X, Location.Y, TouchIndex, Input.dwID);
								MessageHandler->OnTouchStarted(CurrentNativeEventWindowPtr, Location, TouchIndex, 0);
							}
							else
							{
								// TODO: Error handling.
							}
						}
						else if (Input.dwFlags & TOUCHEVENTF_MOVE)
						{
							int32 TouchIndex = GetTouchIndexForID(Input.dwID);
							if (TouchIndex >= 0)
							{
								UE_LOG(LogWindowsDesktop, Verbose, TEXT("OnTouchMoved at (%f, %f), finger %d (system touch id %d)"), Location.X, Location.Y, TouchIndex, Input.dwID);
								MessageHandler->OnTouchMoved(Location, TouchIndex, 0);
							}
						}
						else if (Input.dwFlags & TOUCHEVENTF_UP)
						{
							int32 TouchIndex = GetTouchIndexForID(Input.dwID);
							if (TouchIndex >= 0)
							{
								TouchIDs[TouchIndex] = TOptional<int32>();
								UE_LOG(LogWindowsDesktop, Verbose, TEXT("OnTouchEnded at (%f, %f), finger %d (system touch id %d)"), Location.X, Location.Y, TouchIndex, Input.dwID);
								MessageHandler->OnTouchEnded(Location, TouchIndex, 0);
							}
							else
							{
								// TODO: Error handling.
							}
						}
					}
					CloseTouchInputHandle((HTOUCHINPUT)lParam);
					return 0;
				}
			}*/
			break;
		}
#endif

		// Window focus and activation
		case WM_MOUSEACTIVATE:
		{
			// If the mouse activate isn't in the client area we'll force the WM_ACTIVATE to be EWindowActivation::ActivateByMouse
			// This ensures that clicking menu buttons on the header doesn't generate a WM_ACTIVATE with EWindowActivation::Activate
			// which may cause mouse capture to be taken because is not differentiable from Alt-Tabbing back to the application.
			//TODO: This
			//bForceActivateByMouse = !(LOWORD(lParam) & HTCLIENT);
			return 0;
		}
		break;

		// Window focus and activation
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) & WA_ACTIVE || LOWORD(wParam) & WA_CLICKACTIVE)
			{

			}
			else
			{
				_MouseCaptured = false;
				_MouseShowState = true;
				// Show mouse cursor. Each time ShowCursor(true) is called an internal value is incremented so we 
				// call ShowCursor until the cursor is actually shown (>= 0 value returned by showcursor)
				while (::ShowCursor(true) < 0);

				::ReleaseCapture();
			}

			//TODO: This
			/*EWindowActivation ActivationType;

			if (LOWORD(wParam) & WA_ACTIVE)
			{
				ActivationType = bForceActivateByMouse ? EWindowActivation::ActivateByMouse : EWindowActivation::Activate;
			}
			else if (LOWORD(wParam) & WA_CLICKACTIVE)
			{
				ActivationType = EWindowActivation::ActivateByMouse;
			}
			else
			{
				ActivationType = EWindowActivation::Deactivate;
			}
			bForceActivateByMouse = false;

			UpdateAllModifierKeyStates();

			if (CurrentNativeEventWindowPtr.IsValid())
			{
				BOOL Result = false;
				Result = MessageHandler->OnWindowActivationChanged(CurrentNativeEventWindowPtr.ToSharedRef(), ActivationType);
				return Result ? 0 : 1;
			}*/

			return 1;
		}
		break;

		case WM_ACTIVATEAPP:
			//TODO: This
			//UpdateAllModifierKeyStates();
			//MessageHandler->OnApplicationActivationChanged(!!wParam);
			break;

		case WM_SETTINGCHANGE:
			/*if ((lParam != 0) && (FCString::Strcmp((LPCTSTR)lParam, TEXT("ConvertibleSlateMode")) == 0))
			{
				MessageHandler->OnConvertibleLaptopModeChanged();
			}*/
			break;

		case WM_NCACTIVATE:
		{
			/*if (CurrentNativeEventWindowPtr.IsValid() && !CurrentNativeEventWindowPtr->GetDefinition().HasOSWindowBorder)
			{
				// Unless using the OS window border, intercept calls to prevent non-client area drawing a border upon activation or deactivation
				// Return true to ensure standard activation happens
				return true;
			}*/
		}
		break;

		case WM_NCPAINT:
		{
			/*if (CurrentNativeEventWindowPtr.IsValid() && !CurrentNativeEventWindowPtr->GetDefinition().HasOSWindowBorder)
			{
				// Unless using the OS window border, intercept calls to draw the non-client area - we do this ourselves
				return 0;
			}*/
		}
		break;

		case WM_CLOSE:
		{
			/*if (CurrentNativeEventWindowPtr.IsValid())
			{
				// Called when the OS close button is pressed
				MessageHandler->OnWindowClose(CurrentNativeEventWindowPtr.ToSharedRef());
			}*/
			return 0;
		}
		break;

		case WM_SHOWWINDOW:
		{
			//TODO: This
			/*if (CurrentNativeEventWindowPtr.IsValid())
			{
				switch (lParam)
				{
				case SW_PARENTCLOSING:
					CurrentNativeEventWindowPtr->OnParentWindowMinimized();
					break;
				case SW_PARENTOPENING:
					CurrentNativeEventWindowPtr->OnParentWindowRestored();
					break;
				default:
					break;
				}
			}*/
		}
		break;

		case WM_SIZE:
		{
			//TODO: This
			/*if (true)
			{
				// @todo Fullscreen - Perform deferred resize
				// Note WM_SIZE provides the client dimension which is not equal to the window dimension if there is a windows border 
				const int32 NewWidth = (int)(short)(LOWORD(lParam));
				const int32 NewHeight = (int)(short)(HIWORD(lParam));

				const FGenericWindowDefinition& Definition = CurrentNativeEventWindowPtr->GetDefinition();
				if (Definition.IsRegularWindow && !Definition.HasOSWindowBorder)
				{
					CurrentNativeEventWindowPtr->AdjustWindowRegion(NewWidth, NewHeight);
				}

				const bool bWasMinimized = (wParam == SIZE_MINIMIZED);

				const bool bIsFullscreen = (CurrentNativeEventWindowPtr->GetWindowMode() == EWindowMode::Type::Fullscreen);

				// When in fullscreen Windows rendering size should be determined by the application. Do not adjust based on WM_SIZE messages.
				if (!bIsFullscreen)
				{
					const bool Result = MessageHandler->OnSizeChanged(CurrentNativeEventWindowPtr.ToSharedRef(), NewWidth, NewHeight, bWasMinimized);
				}
			}*/
		}
		break;
		case WM_SIZING:
		{
			//TODO: This
			/*if (CurrentNativeEventWindowPtr.IsValid())
			{
				MessageHandler->OnResizingWindow(CurrentNativeEventWindowPtr.ToSharedRef());
			}*/
		}
		break;
		case WM_ENTERSIZEMOVE:
		{
			//TODO: This
			/*if (CurrentNativeEventWindowPtr.IsValid())
			{
				MessageHandler->BeginReshapingWindow(CurrentNativeEventWindowPtr.ToSharedRef());
			}*/
		}
		break;
		case WM_EXITSIZEMOVE:
		{
			//TODO: This
			/*if (CurrentNativeEventWindowPtr.IsValid())
			{
				MessageHandler->FinishedReshapingWindow(CurrentNativeEventWindowPtr.ToSharedRef());
			}*/
		}
		break;

#if WINVER > 0x502
		case WM_DWMCOMPOSITIONCHANGED:
		{
			//TODO: This
			//CurrentNativeEventWindowPtr->OnTransparencySupportChanged(GetWindowTransparencySupport());
		}
		break;
#endif

		case WM_DPICHANGED:
		{
			//TODO: This
			/*if (CurrentNativeEventWindowPtr.IsValid())
			{
				CurrentNativeEventWindowPtr->SetDPIScaleFactor(LOWORD(wParam) / 96.0f);


				LPRECT NewRect = (LPRECT)lParam;
				SetWindowPos(hwnd, nullptr, NewRect->left, NewRect->top, NewRect->right - NewRect->left, NewRect->bottom - NewRect->top, SWP_NOZORDER | SWP_NOACTIVATE);

				MessageHandler->HandleDPIScaleChanged(CurrentNativeEventWindowPtr.ToSharedRef());
			}*/
		}
		break;
		}

		return S_OK;
	}

	bool WindowsInputManager::IsInputMessage(uint32 msg)
	{
		if (IsKeyboardInputMessage(msg) || IsMouseInputMessage(msg))
		{
			return true;
		}

		switch (msg)
		{
			// Raw input notification messages...
		case WM_INPUT:
		case WM_INPUT_DEVICE_CHANGE:
			return true;
		}
		return false;
	}

	bool WindowsInputManager::IsKeyboardInputMessage(uint32 msg)
	{
		switch (msg)
		{
			// Keyboard input notification messages...
		case WM_CHAR:
		case WM_SYSCHAR:
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYUP:
		case WM_SYSCOMMAND:
			return true;
		}
		return false;
	}

	bool WindowsInputManager::IsMouseInputMessage(uint32 msg)
	{
		switch (msg)
		{
			// Mouse input notification messages...
		case WM_MOUSEHWHEEL:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHOVER:
		case WM_MOUSELEAVE:
		case WM_MOUSEMOVE:
		case WM_NCMOUSEHOVER:
		case WM_NCMOUSELEAVE:
		case WM_NCMOUSEMOVE:
		case WM_NCMBUTTONDBLCLK:
		case WM_NCMBUTTONDOWN:
		case WM_NCMBUTTONUP:
		case WM_NCRBUTTONDBLCLK:
		case WM_NCRBUTTONDOWN:
		case WM_NCRBUTTONUP:
		case WM_NCXBUTTONDBLCLK:
		case WM_NCXBUTTONDOWN:
		case WM_NCXBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_XBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
			return true;
		}
		return false;
	}

	void WindowsInputManager::UpdateAllModifierKeyStates()
	{
		ModifierKeyState[EModifierKey::LeftShift] = (::GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0;
		ModifierKeyState[EModifierKey::RightShift] = (::GetAsyncKeyState(VK_RSHIFT) & 0x8000) != 0;
		ModifierKeyState[EModifierKey::LeftControl] = (::GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
		ModifierKeyState[EModifierKey::RightControl] = (::GetAsyncKeyState(VK_RCONTROL) & 0x8000) != 0;
		ModifierKeyState[EModifierKey::LeftAlt] = (::GetAsyncKeyState(VK_LMENU) & 0x8000) != 0;
		ModifierKeyState[EModifierKey::RightAlt] = (::GetAsyncKeyState(VK_RMENU) & 0x8000) != 0;
		ModifierKeyState[EModifierKey::CapsLock] = (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
	}

	void WindowsInputManager::Update()
	{
		InputManager::Update();

		if (_MouseCaptured)
		{
			_MouseShowState = false;

			if (!_MouseHiddenState)
			{
				_MouseHiddenState = true;
				// Disable the cursor.  Wait until its actually disabled.
				while (::ShowCursor(false) >= 0);
			}

			HWND hWnd = (static_cast<DeviceManagerDX11*>(_Context->GetDeviceManager()))->GetHWND();

			RECT rect;
			GetWindowRect(hWnd, &rect);

			float centerX = rect.left + (rect.right - rect.left) / 2.0f;
			float centerY = rect.top + (rect.bottom - rect.top) / 2.0f;

			::SetCursorPos(static_cast<int>(centerX), static_cast<int>(centerY));

			POINT pos;
			::GetCursorPos(&pos);
			::ScreenToClient(hWnd, &pos);

			_LastMousePos.x = pos.x;
			_LastMousePos.y = pos.y;

			::SetCapture(hWnd);
		}
		else
		{
			_MouseHiddenState = false;

			if (!_MouseShowState)
			{
				_MouseShowState = true;
				// Show mouse cursor. Each time ShowCursor(true) is called an internal value is incremented so we 
				// call ShowCursor until the cursor is actually shown (>= 0 value returned by showcursor)
				while (::ShowCursor(true) < 0);

				::ReleaseCapture();
			}
		}

		//::SetCapture(_deviceManager->GetHWND());


		//::ClipCursor(&rect);
	}
}