#include "Hydra/Render/Pipelines/DX11Window.h"

#ifdef OPERATING_SYSTEM_WINDOWS

#define WINDOW_CLASS_NAME   L"HydraEngine"
#define WINDOW_STYLE_NORMAL         (WS_OVERLAPPEDWINDOW | WS_VISIBLE)
#define WINDOW_STYLE_NORMAL_NO_RESIZE (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
#define WINDOW_STYLE_FULLSCREEN     (WS_POPUP | WS_SYSMENU | WS_VISIBLE)

#include <d3d11.h>
#include <d3d11_1.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

namespace Hydra
{
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DX11Window* pDM = (DX11Window*)GetWindowLongPtr(hWnd, 0);

		if (pDM)
		{
			//return pDM->msgProc(hWnd, uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	DX11Window::DX11Window() : _WindowHandle(NULL), _SwapChain(NULL), _Device(NULL), _ImmediateContext(NULL), _BackBufferRTV(NULL)
	{
	}

	DX11Window::~DX11Window()
	{
		if (IsOpen())
		{
			Close();
		}
	}

	void DX11Window::Open(const WindowState & mode, const String & title, bool resizable)
	{
		Open(1280, 720, mode, title, resizable);
	}

	void DX11Window::Open(int width, int height, const WindowState & mode, const String & title, bool resizable)
	{
		HINSTANCE hInstance = GetModuleHandle(NULL);

		WNDCLASSEX windowClass = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WindowProc, 0L, sizeof(void*), hInstance, NULL, NULL, NULL, NULL, WINDOW_CLASS_NAME, NULL };
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);

		RegisterClassEx(&windowClass);

		bool fullScreen = mode == WindowState::Fullscreen;
		bool maximized = mode == WindowState::Maximized;

		UINT windowStyle = fullScreen
			? WINDOW_STYLE_FULLSCREEN
			: maximized
			? (WINDOW_STYLE_NORMAL | WS_MAXIMIZE)
			: resizable
			? WINDOW_STYLE_NORMAL
			: WINDOW_STYLE_NORMAL_NO_RESIZE;

		RECT rect = { 0, 0, width, height };
		AdjustWindowRect(&rect, windowStyle, FALSE);

		IDXGIAdapter* targetAdapter = FindAdapter(L"", FALSE);

		if (targetAdapter)
		{
			rect = MoveWindowOntoAdapter(targetAdapter, rect);
		}
		else
		{
			LOG("Cannot create window adapter");
			return;
		}

		wchar_t wchTitle[256];
		MultiByteToWideChar(CP_ACP, 0, title.c_str(), -1, wchTitle, 256);

		_WindowHandle = CreateWindowEx(
			0,
			WINDOW_CLASS_NAME,
			wchTitle,
			windowStyle,
			rect.left,
			rect.top,
			rect.right - rect.left,
			rect.bottom - rect.top,
			GetDesktopWindow(),
			NULL,
			hInstance,
			NULL
		);

		if (!_WindowHandle)
		{
#ifdef DEBUG
			DWORD errorCode = GetLastError();
			printf("CreateWindowEx error code = 0x%x\n", errorCode);
#endif
			LOG("Cannot create window");
		}

		SetWindowLongPtr(_WindowHandle, 0, (LONG_PTR)this);
		UpdateWindow(_WindowHandle);

		//Create DXGI swap chain

		ZeroMemory(&_SwapChainDesc, sizeof(_SwapChainDesc));
		_SwapChainDesc.BufferCount = 1;
		_SwapChainDesc.BufferDesc.Width = width;
		_SwapChainDesc.BufferDesc.Height = height;
		_SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		_SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		_SwapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
		_SwapChainDesc.BufferUsage = (DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT);
		_SwapChainDesc.OutputWindow = _WindowHandle;
		_SwapChainDesc.SampleDesc.Count = 1;
		_SwapChainDesc.SampleDesc.Quality = 0;
		_SwapChainDesc.Windowed = !fullScreen;
		_SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		UINT createFlags = 0;
		if (false) // DEBUG
		{
			createFlags |= D3D11_CREATE_DEVICE_DEBUG;
		}

		const D3D_DRIVER_TYPE dType = (targetAdapter) ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE;

		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

		HRESULT hr = E_FAIL;

		hr = D3D11CreateDeviceAndSwapChain(
			targetAdapter,          // pAdapter
			dType,					// DriverType
			NULL,                   // Software
			createFlags,            // Flags
			&featureLevel,   // pFeatureLevels
			1,                      // FeatureLevels
			D3D11_SDK_VERSION,      // SDKVersion
			&_SwapChainDesc,       // pSwapChainDesc
			&_SwapChain,           // ppSwapChain
			&_Device,              // ppDevice
			NULL,                   // pFeatureLevel
			&_ImmediateContext     // ppImmediateContext
		);

		if (targetAdapter)
		{
			targetAdapter->Release();
		}

		if (FAILED(hr))
		{
			LOG("Window creation failed.");
			return;
		}

		hr = CreateRenderTarget();

		if (FAILED(hr))
		{
			LOG("Window creation failed.");
			return;
		}
	}

	IDXGIAdapter * DX11Window::FindAdapter(const WCHAR * targetName, bool isNv)
	{
		IDXGIAdapter* targetAdapter = NULL;
		IDXGIFactory* IDXGIFactory_0001 = NULL;
		HRESULT hres = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&IDXGIFactory_0001);

		if (hres != S_OK)
		{
			printf("ERROR in CreateDXGIFactory, %s@%d.\nFor more info, get log from debug D3D runtime: (1) Install DX SDK, and enable Debug D3D from DX Control Panel Utility. (2) Install and start DbgView. (3) Try running the program again.\n", __FILE__, __LINE__);
			return targetAdapter;
		}

		unsigned int adapterNo = 0;
		while (SUCCEEDED(hres))
		{
			IDXGIAdapter* pAdapter = NULL;
			hres = IDXGIFactory_0001->EnumAdapters(adapterNo, (IDXGIAdapter**)&pAdapter);

			if (SUCCEEDED(hres))
			{
				DXGI_ADAPTER_DESC aDesc;
				pAdapter->GetDesc(&aDesc);

				// If no name is specified, return the first adapater.  This is the same behaviour as the 
				// default specified for D3D11CreateDevice when no adapter is specified.
				if (wcslen(targetName) == 0)
				{
					targetAdapter = pAdapter;
					isNv = aDesc.VendorId == 0x10DE;
					break;
				}

				std::wstring aName = aDesc.Description;
				if (aName.find(targetName) != std::string::npos)
				{
					targetAdapter = pAdapter;
					isNv = aDesc.VendorId == 0x10DE;
				}
				else
				{
					pAdapter->Release();
				}
			}

			adapterNo++;
		}

		if (IDXGIFactory_0001)
		{
			IDXGIFactory_0001->Release();
		}

		return targetAdapter;
	}

	RECT DX11Window::MoveWindowOntoAdapter(IDXGIAdapter * vTargetAdapter, const RECT & vRect)
	{
		assert(vTargetAdapter != NULL);

		RECT result = vRect;
		HRESULT hres = S_OK;
		unsigned int outputNo = 0;
		while (SUCCEEDED(hres))
		{
			IDXGIOutput* pOutput = NULL;
			hres = vTargetAdapter->EnumOutputs(outputNo++, &pOutput);

			if (SUCCEEDED(hres) && pOutput)
			{
				DXGI_OUTPUT_DESC OutputDesc;
				pOutput->GetDesc(&OutputDesc);
				const RECT desktop = OutputDesc.DesktopCoordinates;
				const int centreX = (int)desktop.left + (int)(desktop.right - desktop.left) / 2;
				const int centreY = (int)desktop.top + (int)(desktop.bottom - desktop.top) / 2;
				const int winW = vRect.right - vRect.left;
				const int winH = vRect.bottom - vRect.top;
				int left = centreX - winW / 2;
				int right = left + winW;
				int top = centreY - winH / 2;
				int bottom = top + winH;
				result.left = std::max<int>(left, (int)desktop.left);
				result.right = std::min<int>(right, (int)desktop.right);
				result.bottom = std::min<int>(bottom, (int)desktop.bottom);
				result.top = std::max<int>(top, (int)desktop.top);

				// If there is more than one output, go with the first found.  Multi-monitor support could go here.
				break;
			}
		}
		return result;
	}

	HRESULT DX11Window::CreateRenderTarget()
	{
		HRESULT hr;

		ID3D11Texture2D *backBuffer = NULL;
		hr = _SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);

		if (FAILED(hr))
		{
			return hr;
		}

		hr = _Device->CreateRenderTargetView(backBuffer, NULL, &_BackBufferRTV);
		backBuffer->Release();

		if (FAILED(hr))
		{
			return hr;
		}

		return S_OK;
	}

	void DX11Window::Close()
	{
		if (_WindowHandle)
		{
			DestroyWindow(_WindowHandle);
			_WindowHandle = NULL;
		}
	}

	bool DX11Window::IsOpen()
	{
		return _WindowHandle != NULL;
	}

	void DX11Window::Update()
	{
		MSG msg = { 0 };

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{

				Sleep(0);
			}
		}
	}

	bool DX11Window::GetMonitorResolution(int & width, int & height, int index)
	{
		return false;
	}

	WindowState DX11Window::GetWindowState()
	{
		return WindowState();
	}

	bool DX11Window::EnterFullScreenMode()
	{
		return false;
	}

	bool DX11Window::ExitFullScreenMode()
	{
		return false;
	}

	void DX11Window::Maximize()
	{
	}

	void DX11Window::Minimize()
	{
	}

	void DX11Window::Restore()
	{
	}

	void DX11Window::SetFocus()
	{
	}

	bool DX11Window::IsResizable()
	{
		return false;
	}

	void DX11Window::SetTitle(const String & title)
	{
	}

	String DX11Window::GetTitle()
	{
		return String();
	}

	void DX11Window::SetMouseVisibility(bool visible)
	{
	}

	void DX11Window::SetMousePosition(int x, int y)
	{
	}

	void DX11Window::GetMousePosition(int & x, int & y)
	{
	}

	int DX11Window::GetWidth()
	{
		return 0;
	}

	int DX11Window::GetHeight()
	{
		return 0;
	}

	WindowHandle DX11Window::GetWindowHandle()
	{
		return WindowHandle();
	}
}

#endif