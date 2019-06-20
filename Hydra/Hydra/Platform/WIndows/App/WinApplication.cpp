#include "WinApplication.h"

#include <dwmapi.h>

#include "WinWindow.h"
#include "WinTaskbar.h"

static WinApplication* WinApp;

LRESULT CALLBACK AppWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return WinApp->ProcessMessage(hWnd, uMsg, wParam, lParam);
}

WinApplication::WinApplication() : Application()
{
	Taskbar = MakeTaskbar();

	WinApp = this;

	RegisterClassInstance(GetModuleHandle(NULL), NULL);
}

WinApplication::~WinApplication()
{
	for (SharedPtr<WinWindow> window : Windows)
	{
		window->Destroy();
	}
}

SharedPtr<FWindow> WinApplication::MakeWindow()
{
	return WinWindow::Make();
}

SharedPtr<FTaskbar> WinApplication::MakeTaskbar()
{
	return MakeShared<WinTaskbar>();
}

void WinApplication::InitializeWindow(const SharedPtr<FWindow>& InWindow, const SharedPtr<FWindowDefinition>& InDefinition, const SharedPtr<FWindow>& InParent, const bool bShowImmediately)
{
	if (InWindow == nullptr)
	{
		return;
	}

	const SharedPtr<WinWindow> winWindow = StaticCastSharedPtr<WinWindow, FWindow>(InWindow);
	const SharedPtr<WinWindow> parentWinWindow = StaticCastSharedPtr<WinWindow, FWindow>(InParent);

	Windows.push_back(winWindow);
	winWindow->Initialize(this, InDefinition, _hInstance, parentWinWindow);

	if (bShowImmediately)
	{
		winWindow->Show();
	}
}

bool WinApplication::RegisterClassInstance(const HINSTANCE HInstance, const HICON HIcon)
{
	_hInstance = HInstance;

	WNDCLASS wc;
	Memzero(wc);

	wc.style = CS_DBLCLKS;

	wc.lpfnWndProc = AppWindowProc;

	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = HInstance;
	wc.hIcon = HIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL; // Transparent
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"HydraWindow";

	if (!::RegisterClass(&wc))
	{
		ThrowLastError("Window Registration Failed!");

		return false;
	}

	return true;
}

SharedPtr<WinWindow> WinApplication::FindWindowByHWND(HWND hwnd)
{
	for (SharedPtr<WinWindow> window : Windows)
	{
		if (window->GetHWnd() == hwnd)
		{
			return window;
		}
	}

	return nullptr;
}

int32 WinApplication::ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam)
{
	//std::cout << hwnd << std::endl;

	SharedPtr<WinWindow> window = FindWindowByHWND(hwnd);

	if (window == nullptr)
	{
		return DefWindowProc(hwnd, msg, wParam, lParam);;
	}

	switch (msg)
	{
		case WM_DESTROY:
		{
			List_Remove(Windows, window);
			return 0;
		}
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);;
}

String WinApplication::GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return String_Empty; //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	String message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

void WinApplication::ThrowLastError(const String& title)
{
	String err = GetLastErrorAsString();
	MessageBoxA(NULL, err.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
}

EWindowTransparency WinApplication::GetWindowTransparencySupport() const
{
#if ALPHA_BLENDED_WINDOWS
	BOOL bIsCompositionEnabled = FALSE;
	::DwmIsCompositionEnabled(&bIsCompositionEnabled);

	return bIsCompositionEnabled ? EWindowTransparency::PerPixel : EWindowTransparency::PerWindow;
#else
	return EWindowTransparency::PerWindow;
#endif
}
