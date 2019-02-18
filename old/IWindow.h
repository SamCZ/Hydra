#pragma once

#include "Hydra/Utils/Common.h"

namespace Hydra
{
	enum class WindowState
	{
		Normal, Maximized, Minimized, Fullscreen
	};

	typedef void* WindowHandle;

	class IWindow
	{
	public:
		virtual void Open(const WindowState& mode, const String& title, bool resizable = true) = 0;
		virtual void Open(int width, int height, const WindowState& mode, const String& title, bool resizable = true) = 0;

		virtual void Close() = 0;
		virtual bool IsOpen() = 0;
		virtual void Update() = 0;

		virtual bool GetMonitorResolution(int& width, int& height, int index = 0) = 0;

		virtual WindowState GetWindowState() = 0;

		virtual bool EnterFullScreenMode() = 0;
		virtual bool ExitFullScreenMode() = 0;

		virtual void Maximize() = 0;
		virtual void Minimize() = 0;
		virtual void Restore() = 0;
		virtual void SetFocus() = 0;

		virtual bool IsResizable() = 0;
		
		virtual void SetTitle(const String& title) = 0;
		virtual String GetTitle() = 0;

		virtual void SetMouseVisibility(bool visible) = 0;
		virtual void SetMousePosition(int x, int y) = 0;
		virtual void GetMousePosition(int& x, int& y) = 0;

		virtual int GetWidth() = 0;
		virtual int GetHeight() = 0;

		virtual WindowHandle GetWindowHandle() = 0;

	};
}