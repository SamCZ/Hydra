#include "WinWindow.h"
#include "WinApplication.h"

#include "Hydra/Core/Vector.h"

WinWindow::WinWindow() :
	WindowMode(EWindowMode::Windowed),
	bIsVisible(false),
	bIsFirstTimeVisible(true),
	bInitiallyMinimized(false),
	bInitiallyMaximized(false)
{
}

WinWindow::~WinWindow()
{

}

void WinWindow::Initialize(WinApplication* const Application, const SharedPtr<FWindowDefinition>& InDefinition, HINSTANCE InHInstance, const SharedPtr<WinWindow>& InParent)
{
	Definition = InDefinition;

	uint32 WindowExStyle = 0;
	uint32 WindowStyle = 0;

	RegionWidth = RegionHeight = -1;

	const float XInitialRect = Definition->XDesiredPositionOnScreen;
	const float YInitialRect = Definition->YDesiredPositionOnScreen;

	const float WidthInitial = Definition->WidthDesiredOnScreen;
	const float HeightInitial = Definition->HeightDesiredOnScreen;

	//DPIScaleFactor = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(XInitialRect, YInitialRect);

	int32 ClientX = floor(XInitialRect);
	int32 ClientY = floor(YInitialRect);
	int32 ClientWidth = floor(WidthInitial);
	int32 ClientHeight = floor(HeightInitial);
	int32 WindowX = ClientX;
	int32 WindowY = ClientY;
	int32 WindowWidth = ClientWidth;
	int32 WindowHeight = ClientHeight;
	const bool bApplicationSupportsPerPixelBlending =
#if ALPHA_BLENDED_WINDOWS
		Application->GetWindowTransparencySupport() == EWindowTransparency::PerPixel;
#else
		false;
#endif

	if (!Definition->HasOSWindowBorder)
	{
		WindowExStyle = WS_EX_WINDOWEDGE;

		if (Definition->TransparencySupport == EWindowTransparency::PerWindow)
		{
			WindowExStyle |= WS_EX_LAYERED;
		}
#if ALPHA_BLENDED_WINDOWS
		else if (Definition->TransparencySupport == EWindowTransparency::PerPixel)
		{
			if (bApplicationSupportsPerPixelBlending)
			{
				WindowExStyle |= WS_EX_COMPOSITED;
			}
		}
#endif

		WindowStyle = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		if (Definition->AppearsInTaskbar)
		{
			WindowExStyle |= WS_EX_APPWINDOW;
		}
		else
		{
			WindowExStyle |= WS_EX_TOOLWINDOW;
		}

		if (Definition->IsTopmostWindow)
		{
			// Tool tips are always top most windows
			WindowExStyle |= WS_EX_TOPMOST;
		}

		if (!Definition->AcceptsInput)
		{
			// Window should never get input
			WindowExStyle |= WS_EX_TRANSPARENT;
		}
	}
	else
	{
		// OS Window border setup
		WindowExStyle = WS_EX_APPWINDOW;
		WindowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;

		if (IsRegularWindow())
		{
			if (Definition->SupportsMaximize)
			{
				WindowStyle |= WS_MAXIMIZEBOX;
			}

			if (Definition->SupportsMinimize)
			{
				WindowStyle |= WS_MINIMIZEBOX;
			}

			if (Definition->HasSizingFrame)
			{
				WindowStyle |= WS_THICKFRAME;
			}
			else
			{
				WindowStyle |= WS_BORDER;
			}
		}
		else
		{
			WindowStyle |= WS_POPUP | WS_BORDER;
		}

		// X,Y, Width, Height defines the top-left pixel of the client area on the screen
		// This adjusts a zero rect to give us the size of the border
		RECT BorderRect = { 0, 0, 0, 0 };
		::AdjustWindowRectEx(&BorderRect, WindowStyle, false, WindowExStyle);

		// Border rect size is negative - see MoveWindowTo
		WindowX += BorderRect.left;
		WindowY += BorderRect.top;

		// Inflate the window size by the OS border
		WindowWidth += BorderRect.right - BorderRect.left;
		WindowHeight += BorderRect.bottom - BorderRect.top;
	}

	ToWString(Definition->Title, wTitle);

	HWnd = CreateWindowEx(
		WindowExStyle,
		L"HydraWindow",
		wTitle,
		WindowStyle,
		WindowX, WindowY,
		WindowWidth, WindowHeight,
		(InParent != nullptr) ? static_cast<HWND>(InParent->HWnd) : NULL,
		NULL, InHInstance, NULL);

	if (HWnd == NULL)
	{
		Application->ThrowLastError("Cannot create window");
		return;
	}

#if WINVER >= 0x0601
	if (RegisterTouchWindow(HWnd, 0) == false)
	{
		Application->ThrowLastError("Register touch input failed!");
	}
#endif

	VirtualWidth = ClientWidth;
	VirtualHeight = ClientHeight;

	Reshape(ClientX, ClientY, ClientWidth, ClientHeight);

	if (Definition->TransparencySupport == EWindowTransparency::PerWindow)
	{
		SetOpacity(Definition->Opacity);
	}

	// No region for non regular windows or windows displaying the os window border
	if (IsRegularWindow() && !Definition->HasOSWindowBorder)
	{
		WindowStyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
		if (Definition->SupportsMaximize)
		{
			WindowStyle |= WS_MAXIMIZEBOX;
		}
		if (Definition->SupportsMinimize)
		{
			WindowStyle |= WS_MINIMIZEBOX;
		}
		if (Definition->HasSizingFrame)
		{
			WindowStyle |= WS_THICKFRAME;
		}

		//verify(SetWindowLong(HWnd, GWL_STYLE, WindowStyle));

		uint32 SetWindowPositionFlags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED;

		if (Definition->ActivationPolicy == EWindowActivationPolicy::Never)
		{
			SetWindowPositionFlags |= SWP_NOACTIVATE;
		}

		::SetWindowPos(HWnd, nullptr, 0, 0, 0, 0, SetWindowPositionFlags);

		//AdjustWindowRegion(ClientWidth, ClientHeight);
	}
	else if (Definition->HasOSWindowBorder)
	{
		if (!Definition->HasCloseButton)
		{
			EnableMenuItem(GetSystemMenu(HWnd, false), SC_CLOSE, MF_GRAYED);
		}
	}

	if (IsRegularWindow())
	{
		// Tell OLE that we are opting into drag and drop.
		// Only makes sense for regular windows (windows that last a while.)
		//RegisterDragDrop(HWnd, this);
	}
}

HWND WinWindow::GetHWnd()
{
	return HWnd;
}

SharedPtr<WinWindow> WinWindow::Make()
{
	return MakeShareable(new WinWindow());
}

bool WinWindow::IsRegularWindow() const
{
	return Definition->IsRegularWindow;
}

void WinWindow::AdjustWindowRegion(int32 Width, int32 Height)
{
	RegionWidth = Width;
	RegionHeight = Height;

	HRGN Region = MakeWindowRegionObject(true);

	// NOTE: We explicitly don't delete the Region object, because the OS deletes the handle when it no longer needed after
	// a call to SetWindowRgn.
	SetWindowRgn(HWnd, Region, false);
}

HRGN WinWindow::MakeWindowRegionObject(bool bIncludeBorderWhenMaximized) const
{
	HRGN Region;
	if (RegionWidth != -1 && RegionHeight != INDEX_NONE)
	{
		const bool bIsBorderlessGameWindow = GetDefinition().Type == EWindowType::GameWindow && !GetDefinition().HasOSWindowBorder;
		if (IsMaximized())
		{
			if (bIsBorderlessGameWindow)
			{
				// Windows caches the cxWindowBorders size at window creation. Even if borders are removed or resized Windows will continue to use this value when evaluating regions
				// and sizing windows. When maximized this means that our window position will be offset from the screen origin by (-cxWindowBorders,-cxWindowBorders). We want to
				// display only the region within the maximized screen area, so offset our upper left and lower right by cxWindowBorders.
				WINDOWINFO WindowInfo;
				Memzero(WindowInfo);
				WindowInfo.cbSize = sizeof(WindowInfo);
				::GetWindowInfo(HWnd, &WindowInfo);

				const int32 WindowBorderSize = bIncludeBorderWhenMaximized ? WindowInfo.cxWindowBorders : 0;
				Region = CreateRectRgn(WindowBorderSize, WindowBorderSize, RegionWidth + WindowBorderSize, RegionHeight + WindowBorderSize);
			}
			else
			{
				const int32 WindowBorderSize = bIncludeBorderWhenMaximized ? GetWindowBorderSize() : 0;
				Region = CreateRectRgn(WindowBorderSize, WindowBorderSize, RegionWidth - WindowBorderSize, RegionHeight - WindowBorderSize);
			}
		}
		else
		{
			const bool bUseCornerRadius = WindowMode == EWindowMode::Windowed && !bIsBorderlessGameWindow &&
#if ALPHA_BLENDED_WINDOWS
				// Corner radii cause DWM window composition blending to fail, so we always set regions to full size rectangles
				Definition->TransparencySupport != EWindowTransparency::PerPixel &&
#endif
				Definition->CornerRadius > 0;

			if (bUseCornerRadius)
			{
				// CreateRoundRectRgn gives you a duff region that's 1 pixel smaller than you ask for. CreateRectRgn behaves correctly.
				// This can be verified by uncommenting the assert below
				Region = CreateRoundRectRgn(0, 0, RegionWidth + 1, RegionHeight + 1, Definition->CornerRadius, Definition->CornerRadius);

				// Test that a point that should be in the region, is in the region
				// check(!!PtInRegion(Region, RegionWidth-1, RegionHeight/2));
			}
			else
			{
				Region = CreateRectRgn(0, 0, RegionWidth, RegionHeight);
			}
		}
	}
	else
	{
		RECT rcWnd;
		GetWindowRect(HWnd, &rcWnd);
		Region = CreateRectRgn(0, 0, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top);
	}

	return Region;
}

int32 WinWindow::GetWindowBorderSize() const
{
	if (GetDefinition().Type == EWindowType::GameWindow && !GetDefinition().HasOSWindowBorder)
	{
		return 0;
	}

	WINDOWINFO WindowInfo;
	Memzero(WindowInfo);
	WindowInfo.cbSize = sizeof(WindowInfo);
	::GetWindowInfo(HWnd, &WindowInfo);

	return WindowInfo.cxWindowBorders;
}

FWindowDefinition& WinWindow::GetDefinition() const
{
	return *Definition;
}

void WinWindow::Show()
{
	if (!bIsVisible)
	{
		bIsVisible = true;

		bool bShouldActivate = false;
		if (Definition->AcceptsInput)
		{
			bShouldActivate = Definition->ActivationPolicy == EWindowActivationPolicy::Always;
			if (bIsFirstTimeVisible && Definition->ActivationPolicy == EWindowActivationPolicy::FirstShown)
			{
				bShouldActivate = true;
			}
		}

		// Map to the relevant ShowWindow command.
		int ShowWindowCommand = bShouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
		if (bIsFirstTimeVisible)
		{
			bIsFirstTimeVisible = false;
			if (bInitiallyMinimized)
			{
				ShowWindowCommand = bShouldActivate ? SW_MINIMIZE : SW_SHOWMINNOACTIVE;
			}
			else if (bInitiallyMaximized)
			{
				ShowWindowCommand = bShouldActivate ? SW_SHOWMAXIMIZED : SW_MAXIMIZE;
			}
		}

		::ShowWindow(HWnd, ShowWindowCommand);
	}
}

void WinWindow::Hide()
{
	if (bIsVisible)
	{
		bIsVisible = false;
		::ShowWindow(HWnd, SW_HIDE);
	}
}

void WinWindow::Destroy()
{
	if (HWnd)
	{
		::DestroyWindow(HWnd);
	}
}

void WinWindow::Reshape(int32 NewX, int32 NewY, int32 NewWidth, int32 NewHeight)
{
	WINDOWINFO WindowInfo;
	memset(&WindowInfo, 0, sizeof(WindowInfo));
	WindowInfo.cbSize = sizeof(WindowInfo);
	::GetWindowInfo(HWnd, &WindowInfo);

	AspectRatio = (float)NewWidth / (float)NewHeight;

	// X,Y, Width, Height defines the top-left pixel of the client area on the screen
	if (Definition->HasOSWindowBorder)
	{
		// This adjusts a zero rect to give us the size of the border
		RECT BorderRect = { 0, 0, 0, 0 };
		::AdjustWindowRectEx(&BorderRect, WindowInfo.dwStyle, false, WindowInfo.dwExStyle);

		// Border rect size is negative - see MoveWindowTo
		NewX += BorderRect.left;
		NewY += BorderRect.top;

		// Inflate the window size by the OS border
		NewWidth += BorderRect.right - BorderRect.left;
		NewHeight += BorderRect.bottom - BorderRect.top;
	}

	// the window position is the requested position
	int32 WindowX = NewX;
	int32 WindowY = NewY;

	// If the window size changes often, only grow the window, never shrink it
	const bool bVirtualSizeChanged = NewWidth != VirtualWidth || NewHeight != VirtualHeight;
	VirtualWidth = NewWidth;
	VirtualHeight = NewHeight;

	if (Definition->SizeWillChangeOften)
	{
		// When SizeWillChangeOften is set, we set a minimum width and height window size that we'll keep allocated
		// even when the requested actual window size is smaller.  This just avoids constantly resizing the window
		// and associated GPU buffers, which can be very slow on some platforms.

		const RECT OldWindowRect = WindowInfo.rcWindow;
		const int32 OldWidth = OldWindowRect.right - OldWindowRect.left;
		const int32 OldHeight = OldWindowRect.bottom - OldWindowRect.top;

		const int32 MinRetainedWidth = Definition->ExpectedMaxWidth != -1 ? Definition->ExpectedMaxWidth : OldWidth;
		const int32 MinRetainedHeight = Definition->ExpectedMaxHeight != -1 ? Definition->ExpectedMaxHeight : OldHeight;

		NewWidth = glm::max(NewWidth, glm::min(OldWidth, MinRetainedWidth));
		NewHeight = glm::max(NewHeight, glm::min(OldHeight, MinRetainedHeight));
	}

	if (IsMaximized())
	{
		Restore();
	}

	// We use SWP_NOSENDCHANGING when in fullscreen mode to prevent Windows limiting our window size to the current resolution, as that 
	// prevents us being able to change to a higher resolution while in fullscreen mode
	::SetWindowPos(HWnd, nullptr, WindowX, WindowY, NewWidth, NewHeight, SWP_NOZORDER | SWP_NOACTIVATE | ((WindowMode == EWindowMode::Fullscreen) ? SWP_NOSENDCHANGING : 0));

	if (Definition->SizeWillChangeOften && bVirtualSizeChanged)
	{
		AdjustWindowRegion(VirtualWidth, VirtualHeight);
	}
}

void WinWindow::Minimize()
{
	if (!bIsFirstTimeVisible)
	{
		::ShowWindow(HWnd, SW_MINIMIZE);
	}
	else
	{
		bInitiallyMinimized = true;
		bInitiallyMaximized = false;
	}
}

void WinWindow::Maximize()
{
	if (!bIsFirstTimeVisible)
	{
		::ShowWindow(HWnd, SW_MAXIMIZE);
	}
	else
	{
		bInitiallyMaximized = true;
		bInitiallyMinimized = false;
	}
}

void WinWindow::Restore()
{
	if (!bIsFirstTimeVisible)
	{
		::ShowWindow(HWnd, SW_RESTORE);
	}
	else
	{
		bInitiallyMaximized = false;
		bInitiallyMinimized = false;
	}
}

void WinWindow::SetMode(EWindowMode::Type NewWindowMode)
{
	if (NewWindowMode != WindowMode)
	{
		EWindowMode::Type PreviousWindowMode = WindowMode;
		WindowMode = NewWindowMode;

		const bool bTrueFullscreen = NewWindowMode == EWindowMode::Fullscreen;

		// Setup Win32 Flags to be used for Fullscreen mode
		LONG WindowStyle = GetWindowLong(HWnd, GWL_STYLE);
		const LONG FullscreenModeStyle = WS_POPUP;

		LONG WindowedModeStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
		if (IsRegularWindow())
		{
			if (Definition->SupportsMaximize)
			{
				WindowedModeStyle |= WS_MAXIMIZEBOX;
			}

			if (Definition->SupportsMinimize)
			{
				WindowedModeStyle |= WS_MINIMIZEBOX;
			}

			if (Definition->HasSizingFrame)
			{
				WindowedModeStyle |= WS_THICKFRAME;
			}
			else
			{
				WindowedModeStyle |= WS_BORDER;
			}
		}
		else
		{
			WindowedModeStyle |= WS_POPUP | WS_BORDER;
		}

		// If we're not in fullscreen, make it so
		if (NewWindowMode == EWindowMode::WindowedFullscreen || NewWindowMode == EWindowMode::Fullscreen)
		{
			if (PreviousWindowMode == EWindowMode::Windowed)
			{
				PreFullscreenWindowPlacement.length = sizeof(WINDOWPLACEMENT);
				::GetWindowPlacement(HWnd, &PreFullscreenWindowPlacement);
			}

			// Setup Win32 flags for fullscreen window
			WindowStyle &= ~WindowedModeStyle;
			WindowStyle |= FullscreenModeStyle;

			SetWindowLong(HWnd, GWL_STYLE, WindowStyle);
			::SetWindowPos(HWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

			if (!bTrueFullscreen)
			{
				// Ensure the window is restored if we are going for WindowedFullscreen
				ShowWindow(HWnd, SW_RESTORE);
			}

			// Get the current window position.
			RECT ClientRect;
			GetClientRect(HWnd, &ClientRect);

			// Grab current monitor data for sizing
			HMONITOR Monitor = MonitorFromWindow(HWnd, bTrueFullscreen ? MONITOR_DEFAULTTOPRIMARY : MONITOR_DEFAULTTONEAREST);
			MONITORINFO MonitorInfo;
			MonitorInfo.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(Monitor, &MonitorInfo);

			// Get the target client width to send to ReshapeWindow.
			// Preserve the current res if going to true fullscreen and the monitor supports it and allow the calling code
			// to resize if required.
			// Else, use the monitor's res for windowed fullscreen.
			LONG MonitorWidth = MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left;
			LONG TargetClientWidth = bTrueFullscreen ?
				glm::min(MonitorWidth, ClientRect.right - ClientRect.left) :
				MonitorWidth;

			LONG MonitorHeight = MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top;
			LONG TargetClientHeight = bTrueFullscreen ?
				glm::min(MonitorHeight, ClientRect.bottom - ClientRect.top) :
				MonitorHeight;


			// Resize and position fullscreen window
			Reshape(
				MonitorInfo.rcMonitor.left,
				MonitorInfo.rcMonitor.top,
				TargetClientWidth,
				TargetClientHeight);
		}
		else
		{
			// Windowed:

			// Setup Win32 flags for restored window
			WindowStyle &= ~FullscreenModeStyle;
			WindowStyle |= WindowedModeStyle;
			SetWindowLong(HWnd, GWL_STYLE, WindowStyle);
			::SetWindowPos(HWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

			if (PreFullscreenWindowPlacement.length) // Was PreFullscreenWindowPlacement initialized?
			{
				::SetWindowPlacement(HWnd, &PreFullscreenWindowPlacement);
			}
		}
	}
}

EWindowMode::Type WinWindow::GetMode() const
{
	return WindowMode;
}

bool WinWindow::IsMaximized() const
{
	bool bIsMaximized = !!::IsZoomed(HWnd);
	return bIsMaximized;
}

bool WinWindow::IsMinimized() const
{
	return !!::IsIconic(HWnd);
}

bool WinWindow::IsVisible() const
{
	return bIsVisible;
}

void WinWindow::SetFocus()
{
	if (GetFocus() != HWnd)
	{
		::SetFocus(HWnd);
	}
}

void WinWindow::SetOpacity(float Opacity)
{
	SetLayeredWindowAttributes(HWnd, 0, (int)floor(Opacity * 255.0f), LWA_ALPHA);
}
