#pragma once

#include "Hydra/Render/IWindow.h"

#ifdef OPERATING_SYSTEM_WINDOWS

#include <Windows.h>

struct IDXGIAdapter;

namespace Hydra
{
	class DX11Window : public IWindow
	{
	private:
		HWND _WindowHandle;
		DXGI_SWAP_CHAIN_DESC _SwapChainDesc;
		IDXGISwapChain* _SwapChain;
		ID3D11Device* _Device;
		ID3D11DeviceContext* _ImmediateContext;
		ID3D11RenderTargetView* _BackBufferRTV;

	public:
		DX11Window();
		~DX11Window();

		void Open(const WindowState& mode, const String& title, bool resizable = true);
		void Open(int width, int height, const WindowState& mode, const String& title, bool resizable = true);

		void Close();
		bool IsOpen();
		void Update();

		bool GetMonitorResolution(int& width, int& height, int index = 0);

		WindowState GetWindowState();

		bool EnterFullScreenMode();
		bool ExitFullScreenMode();

		void Maximize();
		void Minimize();
		void Restore();
		void SetFocus();

		bool IsResizable();

		void SetTitle(const String& title);
		String GetTitle();

		void SetMouseVisibility(bool visible);
		void SetMousePosition(int x, int y);
		void GetMousePosition(int& x, int& y);

		int GetWidth();
		int GetHeight();

		WindowHandle GetWindowHandle();

	private:
		IDXGIAdapter* FindAdapter(const WCHAR* targetName, bool isNv);
		RECT MoveWindowOntoAdapter(IDXGIAdapter* vTargetAdapter, const RECT& vRect);
		HRESULT CreateRenderTarget();
	};
}

#endif