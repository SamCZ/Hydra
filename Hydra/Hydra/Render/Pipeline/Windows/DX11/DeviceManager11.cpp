/*
* Copyright (c) 2012-2018, NVIDIA CORPORATION. All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto. Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "DeviceManager11.h"
#include <WinUser.h>
#include <Windows.h>
#include <XInput.h>
#include <assert.h>
#include <sstream>
#include <algorithm>

#include <ShObjIdl.h> //Taskbar

#include "Hydra/Core/Timing.h"
#include "VisualController11.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

#define WINDOW_CLASS_NAME   L"NvDX11"

#define WINDOW_STYLE_NORMAL         (WS_OVERLAPPEDWINDOW | WS_VISIBLE)
#define WINDOW_STYLE_FULLSCREEN     (WS_POPUP | WS_SYSMENU | WS_VISIBLE)

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DeviceManagerDX11* pDM = (DeviceManagerDX11*)GetWindowLongPtr(hWnd, 0);
	if (pDM)
		return pDM->MsgProc(hWnd, uMsg, wParam, lParam);
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool IsNvDeviceID(UINT id)
{
	return id == 0x10DE;
}

// Find an adapter whose name contains the given string.
IDXGIAdapter* FindAdapter(const WCHAR* targetName, bool& isNv)
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
				isNv = IsNvDeviceID(aDesc.VendorId);
				break;
			}

			std::wstring aName = aDesc.Description;
			if (aName.find(targetName) != std::string::npos)
			{
				targetAdapter = pAdapter;
				isNv = IsNvDeviceID(aDesc.VendorId);
			}
			else
			{
				pAdapter->Release();
			}
		}

		adapterNo++;
	}

	if (IDXGIFactory_0001)
		IDXGIFactory_0001->Release();

	return targetAdapter;
}

// Adjust window rect so that it is centred on the given adapter.  Clamps to fit if it's too big.
RECT MoveWindowOntoAdapter(IDXGIAdapter* targetAdapter, const RECT& rect)
{
	assert(targetAdapter != NULL);

	RECT result = rect;
	HRESULT hres = S_OK;
	unsigned int outputNo = 0;
	while (SUCCEEDED(hres))
	{
		IDXGIOutput* pOutput = NULL;
		hres = targetAdapter->EnumOutputs(outputNo++, &pOutput);

		if (SUCCEEDED(hres) && pOutput)
		{
			DXGI_OUTPUT_DESC OutputDesc;
			pOutput->GetDesc(&OutputDesc);
			const RECT desktop = OutputDesc.DesktopCoordinates;
			const int centreX = (int)desktop.left + (int)(desktop.right - desktop.left) / 2;
			const int centreY = (int)desktop.top + (int)(desktop.bottom - desktop.top) / 2;
			const int winW = rect.right - rect.left;
			const int winH = rect.bottom - rect.top;
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

HRESULT
DeviceManagerDX11::CreateWindowDeviceAndSwapChain(const DeviceCreationParametersDX11& params, LPWSTR title)
{
	m_WindowTitle = title;

	HINSTANCE hInstance = GetModuleHandle(NULL);
	WNDCLASSEX windowClass = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WindowProc,
						0L, sizeof(void*), hInstance, NULL, NULL, NULL, NULL, WINDOW_CLASS_NAME, NULL };

	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&windowClass);

	UINT windowStyle = params.FullScreen
		? WINDOW_STYLE_FULLSCREEN
		: params.Maximized
		? (WINDOW_STYLE_NORMAL | WS_MAXIMIZE)
		: WINDOW_STYLE_NORMAL;

	RECT rect = { 0, 0, params.Width, params.Height };
	AdjustWindowRect(&rect, windowStyle, FALSE);

	IDXGIAdapter* targetAdapter = FindAdapter(params.adapterNameSubstring, m_IsNvidia);
	if (targetAdapter)
	{
		rect = MoveWindowOntoAdapter(targetAdapter, rect);
	}
	else
	{
		// We could silently use a default adapter in this case.  I think it's better to choke.
		std::wostringstream ostr;
		ostr << L"Could not find an adapter matching \"" << params.adapterNameSubstring << "\"" << std::ends;
		MessageBox(NULL, ostr.str().c_str(), m_WindowTitle.c_str(), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	m_hWnd = CreateWindowEx(
		0,
		WINDOW_CLASS_NAME,
		title,
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

	if (!m_hWnd)
	{
#ifdef DEBUG
		DWORD errorCode = GetLastError();
		printf("CreateWindowEx error code = 0x%x\n", errorCode);
#endif

		MessageBox(NULL, L"Cannot create window", m_WindowTitle.c_str(), MB_OK | MB_ICONERROR);
		return E_FAIL;
	}

	SetWindowLongPtr(m_hWnd, 0, (LONG_PTR)this);
	UpdateWindow(m_hWnd);

	HRESULT hr = E_FAIL;

	RECT clientRect;
	GetClientRect(m_hWnd, &clientRect);
	UINT width = clientRect.right - clientRect.left;
	UINT height = clientRect.bottom - clientRect.top;

	ZeroMemory(&m_SwapChainDesc, sizeof(m_SwapChainDesc));
	m_SwapChainDesc.BufferCount = params.SwapChainBufferCount;
	m_SwapChainDesc.BufferDesc.Width = width;
	m_SwapChainDesc.BufferDesc.Height = height;
	m_SwapChainDesc.BufferDesc.Format = params.SwapChainFormat;
	m_SwapChainDesc.BufferDesc.RefreshRate.Numerator = params.RefreshRate;
	m_SwapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	m_SwapChainDesc.BufferUsage = params.SwapChainUsage;
	m_SwapChainDesc.OutputWindow = m_hWnd;
	m_SwapChainDesc.SampleDesc.Count = params.SwapChainSampleCount;
	m_SwapChainDesc.SampleDesc.Quality = params.SwapChainSampleQuality;
	m_SwapChainDesc.Windowed = !params.FullScreen;
	m_SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	UINT createFlags = 0;
	if (params.EnableDebugRuntime)
		createFlags |= D3D11_CREATE_DEVICE_DEBUG;

	const D3D_DRIVER_TYPE dType = (targetAdapter) ? D3D_DRIVER_TYPE_UNKNOWN : params.DriverType;

	hr = D3D11CreateDeviceAndSwapChain(
		targetAdapter,          // pAdapter
		dType,					// DriverType
		NULL,                   // Software
		createFlags,            // Flags
		&params.FeatureLevel,   // pFeatureLevels
		1,                      // FeatureLevels
		D3D11_SDK_VERSION,      // SDKVersion
		&m_SwapChainDesc,       // pSwapChainDesc
		&m_SwapChain,           // ppSwapChain
		&m_Device,              // ppDevice
		NULL,                   // pFeatureLevel
		&m_ImmediateContext     // ppImmediateContext
	);

	if (targetAdapter)
		targetAdapter->Release();

	if (FAILED(hr))
		return hr;

	hr = CreateRenderTarget();

	if (FAILED(hr))
		return hr;

	DeviceCreated();
	BackBufferResized();

	return S_OK;
}

void
DeviceManagerDX11::Shutdown()
{
	if (m_ShutdownCalled)
		return;

	m_ShutdownCalled = true;

	if (m_SwapChain && GetWindowState() == kWindowFullscreen)
		m_SwapChain->SetFullscreenState(false, NULL);

	DeviceDestroyed();

	SAFE_RELEASE(m_BackBufferRTV);

	SAFE_RELEASE(m_SwapChain);
	SAFE_RELEASE(m_Device);

	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}
}

HRESULT
DeviceManagerDX11::CreateRenderTarget()
{
	HRESULT hr;

	ID3D11Texture2D *backBuffer = NULL;
	hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	if (FAILED(hr))
		return hr;

	hr = m_Device->CreateRenderTargetView(backBuffer, NULL, &m_BackBufferRTV);
	backBuffer->Release();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void
DeviceManagerDX11::MessageLoop()
{
	MSG msg = { 0 };

	LARGE_INTEGER perfFreq, previousTime;
	QueryPerformanceFrequency(&perfFreq);
	QueryPerformanceCounter(&previousTime);

	uint32 fps = 0;
	double lastTime = Time::getTime();
	double fpsTimeCounter = 0.0f;
	double updateTimer = 1.0f;
	float frameTime = 1.0f / 120.0f;

	while (WM_QUIT != msg.message)
	{
#if ENABLE_XINPUT
		XINPUT_KEYSTROKE xInputKeystroke;
		memset(&xInputKeystroke, 0, sizeof(xInputKeystroke));
		const int gamepadIndex = 0;

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (XInputGetKeystroke(gamepadIndex, 0, &xInputKeystroke) == ERROR_SUCCESS && xInputKeystroke.Flags != 0)
		{
			if (xInputKeystroke.Flags & (XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_REPEAT))
			{
				MsgProc(m_hWnd, WM_KEYDOWN, xInputKeystroke.VirtualKey, 0);
			}

			if (xInputKeystroke.Flags & XINPUT_KEYSTROKE_KEYUP)
			{
				MsgProc(m_hWnd, WM_KEYUP, xInputKeystroke.VirtualKey, 0);
			}
		}
#else
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
#endif
		else
		{
			LARGE_INTEGER newTime;
			QueryPerformanceCounter(&newTime);

			double elapsedSeconds = (m_FixedFrameInterval >= 0)
				? m_FixedFrameInterval
				: (double)(newTime.QuadPart - previousTime.QuadPart) / (double)perfFreq.QuadPart;


			if (m_SwapChain && GetWindowState() != kWindowMinimized)
			{
				if (true)
				{
					double currentTime = Time::getTime();
					double passedTime = currentTime - lastTime;
					lastTime = currentTime;

					fpsTimeCounter += passedTime;
					updateTimer += passedTime;

					if (fpsTimeCounter >= 2.0)
					{
						double msPerFrame = 1000.0 / (double)fps;
						//printf("%f ms (%d fps)\r\n", msPerFrame, fps);
						//DEBUG_LOG("FPS", "NONE", "%f ms (%d fps)", msPerFrame, fps);

						SetWindowTextA(m_hWnd, ("Hydra | DX11 | " + ToString(msPerFrame) + " ms (" + ToString(fps) + " fps)").c_str());

						M_MsPerFrame = msPerFrame;
						m_Fps = fps;

						fpsTimeCounter = 0;
						fps = 0;
					}

					bool shouldRender = false;
					while (updateTimer >= frameTime)
					{
						//Update game
						Animate(elapsedSeconds);

						updateTimer -= frameTime;
						shouldRender = true;
					}

					if (shouldRender)
					{
						//Render game

						Render();
						m_SwapChain->Present(m_SyncInterval, 0);
						Sleep(0);

						fps++;
					}
					else
					{
						Time::sleep(1);
					}
				}
				else
				{
					Animate(elapsedSeconds);
					Render();
					m_SwapChain->Present(m_SyncInterval, 0);
					Sleep(0);
				}


			}
			else
			{
				// Release CPU resources when idle
				Sleep(1);
			}

			{
				m_vFrameTimes.push_back(elapsedSeconds);
				double timeSum = 0;
				for (auto it = m_vFrameTimes.begin(); it != m_vFrameTimes.end(); it++)
					timeSum += *it;

				if (timeSum > m_AverageTimeUpdateInterval)
				{
					m_AverageFrameTime = timeSum / (double)m_vFrameTimes.size();
					m_vFrameTimes.clear();
				}
			}

			previousTime = newTime;
		}
	}
}

LRESULT DeviceManagerDX11::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
		case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

		case WM_SYSKEYDOWN:
		if (wParam == VK_F4)
		{
			PostQuitMessage(0);
			return 0;
		}
		break;

		case WM_ENTERSIZEMOVE:
		m_InSizingModalLoop = true;
		m_NewWindowSize.cx = m_SwapChainDesc.BufferDesc.Width;
		m_NewWindowSize.cy = m_SwapChainDesc.BufferDesc.Height;
		break;

		case WM_EXITSIZEMOVE:
		m_InSizingModalLoop = false;
		ResizeSwapChain();
		break;

		case WM_SIZE:
		// Ignore the WM_SIZE event if there is no device,
		// or if the window has been minimized (size == 0),
		// or if it has been restored to the previous size (this part is tested inside ResizeSwapChain)
		if (m_Device && (lParam != 0))
		{
			m_NewWindowSize.cx = LOWORD(lParam);
			m_NewWindowSize.cy = HIWORD(lParam);

			if (!m_InSizingModalLoop)
				ResizeSwapChain();
		}
	}

	if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST ||
		uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST || uMsg == WM_ACTIVATE)
	{
		// processing messages front-to-back
		for (auto it = m_vControllers.begin(); it != m_vControllers.end(); it++)
		{
			if ((*it)->IsEnabled())
			{
				// for kb/mouse messages, 0 means the message has been handled
				if (0 == (*it)->MsgProc(hWnd, uMsg, wParam, lParam))
					return 0;
			}
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void DeviceManagerDX11::ResizeSwapChain()
{
	if (m_NewWindowSize.cx == (LONG)m_SwapChainDesc.BufferDesc.Width &&
		m_NewWindowSize.cy == (LONG)m_SwapChainDesc.BufferDesc.Height)
		return;

	m_SwapChainDesc.BufferDesc.Width = m_NewWindowSize.cx;
	m_SwapChainDesc.BufferDesc.Height = m_NewWindowSize.cy;

	ID3D11RenderTargetView *nullRTV = NULL;
	m_ImmediateContext->OMSetRenderTargets(1, &nullRTV, NULL);
	SAFE_RELEASE(m_BackBufferRTV);

	if (m_SwapChain)
	{
		// Resize the swap chain
		m_SwapChain->ResizeBuffers(m_SwapChainDesc.BufferCount, m_SwapChainDesc.BufferDesc.Width,
			m_SwapChainDesc.BufferDesc.Height, m_SwapChainDesc.BufferDesc.Format,
			m_SwapChainDesc.Flags);

		CreateRenderTarget();

		BackBufferResized();
	}
}

void DeviceManagerDX11::Render()
{
	D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (float)m_SwapChainDesc.BufferDesc.Width, (float)m_SwapChainDesc.BufferDesc.Height, 0.0f, 1.0f };

	if (m_EnableRenderTargetClear)
	{
		m_ImmediateContext->ClearRenderTargetView(m_BackBufferRTV, m_RenderTargetClearColor);
	}

	// rendering back-to-front
	//TODO: Find why i must use begin instead of rbegin and rend
	for (auto it = m_vControllers.begin(); it != m_vControllers.end(); it++)
	{
		if ((*it)->IsEnabled())
		{
			m_ImmediateContext->OMSetRenderTargets(1, &m_BackBufferRTV, nullptr);
			m_ImmediateContext->RSSetViewports(1, &viewport);

			(*it)->Render(m_BackBufferRTV);
		}
	}

	m_ImmediateContext->OMSetRenderTargets(0, NULL, NULL);
}

namespace ETaskbarProgressState
{
	enum Type
	{
		//Stops displaying progress and returns the button to its normal state.
		NoProgress = 0x0,

		//The progress indicator does not grow in size, but cycles repeatedly along the 
		//length of the task bar button. This indicates activity without specifying what 
		//proportion of the progress is complete. Progress is taking place, but there is 
		//no prediction as to how long the operation will take.
		Indeterminate = 0x1,

		//The progress indicator grows in size from left to right in proportion to the 
		//estimated amount of the operation completed. This is a determinate progress 
		//indicator; a prediction is being made as to the duration of the operation.
		Normal = 0x2,

		//The progress indicator turns red to show that an error has occurred in one of 
		//the windows that is broadcasting progress. This is a determinate state. If the 
		//progress indicator is in the indeterminate state, it switches to a red determinate 
		//display of a generic percentage not indicative of actual progress.
		Error = 0x4,

		//The progress indicator turns yellow to show that progress is currently stopped in 
		//one of the windows but can be resumed by the user. No error condition exists and 
		//nothing is preventing the progress from continuing. This is a determinate state. 
		//If the progress indicator is in the indeterminate state, it switches to a yellow 
		//determinate display of a generic percentage not indicative of actual progress.
		Paused = 0x8,
	};
}


static ITaskbarList3* TaskBarList3;

void DeviceManagerDX11::Animate(double fElapsedTimeSeconds)
{
	// front-to-back, but the order shouldn't matter
	for (auto it = m_vControllers.begin(); it != m_vControllers.end(); it++)
	{
		if ((*it)->IsEnabled())
		{
			(*it)->Animate(fElapsedTimeSeconds);
		}
	}
}

void DeviceManagerDX11::DeviceCreated()
{
	::CoInitialize(NULL);

	if (CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, (void **)&TaskBarList3) != S_OK)
	{
		TaskBarList3 = nullptr;
	}

	TaskBarList3->SetProgressState(m_hWnd, (TBPFLAG)ETaskbarProgressState::NoProgress);


	//TaskBarList3->SetProgressValue(m_hWnd, (ULONGLONG)25, (ULONGLONG)100);

	// creating resources front-to-back
	for (auto it = m_vControllers.begin(); it != m_vControllers.end(); it++)
	{
		(*it)->DeviceCreated();
	}
}

void DeviceManagerDX11::DeviceDestroyed()
{
	if (TaskBarList3)
	{
		TaskBarList3->Release();
	}

	// releasing resources back-to-front
	for (auto it = m_vControllers.rbegin(); it != m_vControllers.rend(); it++)
	{
		(*it)->DeviceDestroyed();
	}
}

void DeviceManagerDX11::BackBufferResized()
{
	if (m_SwapChain == NULL)
		return;

	for (auto it = m_vControllers.begin(); it != m_vControllers.end(); it++)
	{
		(*it)->BackBufferResized(m_SwapChainDesc.BufferDesc.Width, m_SwapChainDesc.BufferDesc.Height, m_SwapChainDesc.SampleDesc.Count);
	}
}

HRESULT DeviceManagerDX11::ChangeBackBufferFormat(DXGI_FORMAT format, UINT sampleCount)
{
	HRESULT hr = E_FAIL;

	if ((format == DXGI_FORMAT_UNKNOWN || format == m_SwapChainDesc.BufferDesc.Format) &&
		(sampleCount == 0 || sampleCount == m_SwapChainDesc.SampleDesc.Count))
		return S_FALSE;

	if (m_Device)
	{
		bool fullscreen = (GetWindowState() == kWindowFullscreen);
		if (fullscreen)
			m_SwapChain->SetFullscreenState(false, NULL);

		IDXGISwapChain* newSwapChain = NULL;
		DXGI_SWAP_CHAIN_DESC newSwapChainDesc = m_SwapChainDesc;

		if (format != DXGI_FORMAT_UNKNOWN)
			newSwapChainDesc.BufferDesc.Format = format;
		if (sampleCount != 0)
			newSwapChainDesc.SampleDesc.Count = sampleCount;

		IDXGIAdapter* pDXGIAdapter = GetDXGIAdapter();

		IDXGIFactory* pDXGIFactory = NULL;
		pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pDXGIFactory));

		hr = pDXGIFactory->CreateSwapChain(m_Device, &newSwapChainDesc, &newSwapChain);

		pDXGIFactory->Release();
		pDXGIAdapter->Release();

		if (FAILED(hr))
		{
			if (fullscreen)
				m_SwapChain->SetFullscreenState(true, NULL);

			return hr;
		}

		SAFE_RELEASE(m_BackBufferRTV);
		SAFE_RELEASE(m_SwapChain);

		m_SwapChain = newSwapChain;
		m_SwapChainDesc = newSwapChainDesc;

		if (fullscreen)
			m_SwapChain->SetFullscreenState(true, NULL);

		CreateRenderTarget();
		BackBufferResized();
	}

	return S_OK;
}

void DeviceManagerDX11::AddControllerToFront(IVisualControllerDX11* pController)
{
	m_vControllers.remove(pController);
	m_vControllers.push_front(pController);
}

void DeviceManagerDX11::AddControllerToBack(IVisualControllerDX11* pController)
{
	m_vControllers.remove(pController);
	m_vControllers.push_back(pController);
}

void DeviceManagerDX11::RemoveController(IVisualControllerDX11* pController)
{
	m_vControllers.remove(pController);
}

HRESULT DeviceManagerDX11::ResizeWindow(int width, int height)
{
	if (m_SwapChain == NULL)
		return E_FAIL;

	RECT rect;
	GetWindowRect(m_hWnd, &rect);

	ShowWindow(m_hWnd, SW_RESTORE);

	if (!MoveWindow(m_hWnd, rect.left, rect.top, width, height, true))
		return E_FAIL;

	// No need to call m_SwapChain->ResizeBackBuffer because MoveWindow will send WM_SIZE, which calls that function.

	return S_OK;
}

HRESULT DeviceManagerDX11::EnterFullscreenMode(int width, int height)
{
	if (m_SwapChain == NULL)
		return E_FAIL;

	if (GetWindowState() == kWindowFullscreen)
		return S_FALSE;

	if (width <= 0 || height <= 0)
	{
		width = m_SwapChainDesc.BufferDesc.Width;
		height = m_SwapChainDesc.BufferDesc.Height;
	}

	SetWindowLong(m_hWnd, GWL_STYLE, WINDOW_STYLE_FULLSCREEN);
	MoveWindow(m_hWnd, 0, 0, width, height, true);

	HRESULT hr = m_SwapChain->SetFullscreenState(true, NULL);

	if (FAILED(hr))
	{
		SetWindowLong(m_hWnd, GWL_STYLE, WINDOW_STYLE_NORMAL);
		return hr;
	}

	UpdateWindow(m_hWnd);
	m_SwapChain->GetDesc(&m_SwapChainDesc);

	return S_OK;
}

HRESULT DeviceManagerDX11::LeaveFullscreenMode(int windowWidth, int windowHeight)
{
	if (m_SwapChain == NULL)
		return E_FAIL;

	if (GetWindowState() != kWindowFullscreen)
		return S_FALSE;

	HRESULT hr = m_SwapChain->SetFullscreenState(false, NULL);
	if (FAILED(hr)) return hr;

	SetWindowLong(m_hWnd, GWL_STYLE, WINDOW_STYLE_NORMAL);

	if (windowWidth <= 0 || windowHeight <= 0)
	{
		windowWidth = m_SwapChainDesc.BufferDesc.Width;
		windowHeight = m_SwapChainDesc.BufferDesc.Height;
	}

	RECT rect = { 0, 0, windowWidth, windowHeight };
	AdjustWindowRect(&rect, WINDOW_STYLE_NORMAL, FALSE);
	MoveWindow(m_hWnd, 0, 0, rect.right - rect.left, rect.bottom - rect.top, true);
	UpdateWindow(m_hWnd);

	m_SwapChain->GetDesc(&m_SwapChainDesc);

	return S_OK;
}

HRESULT DeviceManagerDX11::ToggleFullscreen()
{
	if (GetWindowState() == kWindowFullscreen)
		return LeaveFullscreenMode();
	else
		return EnterFullscreenMode();
}

void DeviceManagerDX11::InitContext()
{
	DeviceCreationParametersDX11 params = {};

	params.SwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	params.SwapChainSampleCount = 1;
	params.SwapChainBufferCount = 4;

	OnPrepareDeviceContext.Invoke(params);

	float color[4] = { params.ClearColor.r, params.ClearColor.g, params.ClearColor.b, params.ClearColor.a };
	SetPrimaryRenderTargetClearColor(true, color);

	ToWString(params.Title, wstr);

	if (FAILED(CreateWindowDeviceAndSwapChain(params, wstr)))
	{
		MessageBox(NULL, L"Cannot initialize the DirextX11 device with the requested parameters", L"Error", MB_OK | MB_ICONERROR);
		return;
	}

	MessageLoop();
	Shutdown();
}

void DeviceManagerDX11::AddVisualController(IVisualController* view)
{
	AddControllerToBack(new VisualController(view));
}

DeviceManagerDX11::WindowState DeviceManagerDX11::GetWindowState()
{
	if (m_SwapChain && !m_SwapChainDesc.Windowed)
		return kWindowFullscreen;

	if (m_hWnd == INVALID_HANDLE_VALUE)
		return kWindowNone;

	if (IsZoomed(m_hWnd))
		return kWindowMaximized;

	if (IsIconic(m_hWnd))
		return kWindowMinimized;

	return kWindowNormal;
}

HRESULT DeviceManagerDX11::GetDisplayResolution(int& width, int& height)
{
	if (m_hWnd != INVALID_HANDLE_VALUE)
	{
		HMONITOR monitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO info;
		info.cbSize = sizeof(MONITORINFO);

		if (GetMonitorInfo(monitor, &info))
		{
			width = info.rcMonitor.right - info.rcMonitor.left;
			height = info.rcMonitor.bottom - info.rcMonitor.top;
			return S_OK;
		}
	}

	return E_FAIL;
}

IDXGIAdapter* DeviceManagerDX11::GetDXGIAdapter()
{
	if (!m_Device)
		return NULL;

	IDXGIDevice* pDXGIDevice = NULL;
	m_Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&pDXGIDevice));

	IDXGIAdapter* pDXGIAdapter = NULL;
	pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&pDXGIAdapter));

	pDXGIDevice->Release();

	return pDXGIAdapter;
}

void DeviceManagerDX11::SetPrimaryRenderTargetClearColor(bool enableClear, const float * pColor)
{
	m_EnableRenderTargetClear = enableClear;

	if (pColor)
		memcpy(m_RenderTargetClearColor, pColor, sizeof(float) * 4);
}