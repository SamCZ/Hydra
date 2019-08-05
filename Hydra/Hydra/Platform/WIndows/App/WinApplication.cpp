#include "WinApplication.h"

#include <dwmapi.h>

#include "WinWindow.h"
#include "WinTaskbar.h"

#include "D3DWindowRender.h"

#include "Hydra/EngineContext.h"

#include "Hydra/Assets/AssetManager.h"
#include "Hydra/Render/Graphics.h"

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

}

SharedPtr<FWindow> WinApplication::MakeWindow()
{
	return WinWindow::Make();
}

SharedPtr<FWindowRender> WinApplication::MakeWindowRenderer()
{
	return MakeShared<D3DWindowRender>();
}

SharedPtr<FTaskbar> WinApplication::MakeTaskbar()
{
	return MakeShared<WinTaskbar>();
}

void WinApplication::InitializeWindow(const SharedPtr<FWindow>& InWindow, const SharedPtr<FWindowDefinition>& InDefinition, const SharedPtr<FWindow>& InParent)
{
	if (InWindow == nullptr)
	{
		return;
	}

	const SharedPtr<WinWindow> winWindow = StaticCastSharedPtr<WinWindow, FWindow>(InWindow);
	const SharedPtr<WinWindow> parentWinWindow = StaticCastSharedPtr<WinWindow, FWindow>(InParent);

	Windows.push_back(winWindow);
	winWindow->Initialize(this, InDefinition, _hInstance, parentWinWindow);
}

void WinApplication::Run()
{
	MSG msg = { 0 };

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		IUWindowManager->Tick(0);

		IUWindowManager->Render();
	}
}

void WinApplication::InitializeEngineContext(EngineContext* context)
{
	Context = context;

	D3DWindowRender& windowRenderer = static_cast<D3DWindowRender&>(IUWindowManager->GetRenderer());

	Context->SetRenderInterface(windowRenderer.GetRenderInterface());

	Context->SetAssetManager(new AssetManager(context));

	Context->SetGraphics(new FGraphics(context));
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
			OnWindowDestroy.Invoke(window);

			List_Remove(Windows, window);

			if (Windows.size() == 0)
			{
				PostQuitMessage(0);

				return 0;
			}

			break;
		}

		case WM_SIZE:
		{
			IUWindowManager->GetRenderer().ResizeViewPort(IUWindowManager->FindUIWindowByNativeWindow(StaticCastSharedPtr<FWindow, WinWindow>(window)), LOWORD(lParam), HIWORD(lParam));

			break;
		}
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
