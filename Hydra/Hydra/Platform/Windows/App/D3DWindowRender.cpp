#include "D3DWindowRender.h"

#include "Hydra/App/UI/UIWindow.h"
#include "Hydra/App/Window.h"

D3DWindowRender::D3DWindowRender() : GD3DDevice(nullptr), GD3DDeviceContext(nullptr)
{
	CreateDevice();
}

D3DWindowRender::~D3DWindowRender()
{
	GD3DDeviceContext->Release();
	GD3DDevice->Release();
}

bool D3DWindowRender::CreateDevice()
{
	bool bResult = true;

	if (GD3DDevice == nullptr || GD3DDeviceContext == nullptr)
	{
		// Init D3D
		uint32 DeviceCreationFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
		D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;

		if (false) // DEBUG
		{
			DeviceCreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
		}

		const D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3 };
		D3D_FEATURE_LEVEL CreatedFeatureLevel;
		HRESULT Hr = D3D11CreateDevice(NULL, DriverType, NULL, DeviceCreationFlags, FeatureLevels, sizeof(FeatureLevels) / sizeof(D3D_FEATURE_LEVEL), D3D11_SDK_VERSION, &GD3DDevice, &CreatedFeatureLevel, &GD3DDeviceContext);

		if (FAILED(Hr))
		{
			bResult = false;
			Log("Error");
		}
	}

	return bResult;
}

void D3DWindowRender::CreateBackBufferResources(IDXGISwapChain * InSwapChain, ID3D11Texture2D *& OutBackBuffer, ID3D11RenderTargetView *& OutRTV)
{
	InSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&OutBackBuffer);

	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	RTVDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;
	HRESULT Hr = GD3DDevice->CreateRenderTargetView(OutBackBuffer, &RTVDesc, &OutRTV);
	if (!SUCCEEDED(Hr))
	{
		return;
	}
}

void D3DWindowRender::CreateViewport(SharedPtr<UIWindow>& window)
{
	Vector2i size = window->GetSize();

	bool bFullscreen = window->GetNativeWindow()->GetMode() == EWindowMode::Fullscreen;
	bool bWindowed = true;;

	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	Memzero(SwapChainDesc);
	SwapChainDesc.BufferCount = 1;
	SwapChainDesc.BufferDesc.Width = size.x;
	SwapChainDesc.BufferDesc.Height = size.y;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = (HWND)window->GetNativeWindow()->GetOSWindowHandle();
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.Windowed = bWindowed;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	IDXGIDevice1* DXGIDevice;
	HRESULT Hr = GD3DDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&DXGIDevice);
	if (!SUCCEEDED(Hr))
	{
		return;
	}

	IDXGIAdapter1* DXGIAdapter;
	Hr = DXGIDevice->GetParent(__uuidof(IDXGIAdapter1), (void **)&DXGIAdapter);
	if (!SUCCEEDED(Hr))
	{
		return;
	}

	IDXGIFactory1* DXGIFactory;
	Hr = DXGIAdapter->GetParent(__uuidof(IDXGIFactory1), (void **)&DXGIFactory);
	if (!SUCCEEDED(Hr))
	{
		return;
	}

	SharedPtr<FWindowViewPort> Viewport = MakeShared<FWindowViewPort>();

	Hr = DXGIFactory->CreateSwapChain(DXGIDevice, &SwapChainDesc, &Viewport->D3DSwapChain);
	if (!SUCCEEDED(Hr))
	{
		return;
	}

	Hr = DXGIFactory->MakeWindowAssociation((HWND)window->GetNativeWindow()->GetOSWindowHandle(), DXGI_MWA_NO_ALT_ENTER);
	if (!SUCCEEDED(Hr))
	{
		return;
	}

	uint32 Width = size.x;
	uint32 Height = size.y;

	Viewport->ViewportInfo.MaxDepth = 1.0f;
	Viewport->ViewportInfo.MinDepth = 0.0f;
	Viewport->ViewportInfo.Width = Width;
	Viewport->ViewportInfo.Height = Height;
	Viewport->ViewportInfo.TopLeftX = 0;
	Viewport->ViewportInfo.TopLeftY = 0;

	CreateBackBufferResources(Viewport->D3DSwapChain, Viewport->BackBufferTexture, Viewport->RenderTargetView);

	window->SetViewPort(Viewport);

	//Viewport.ProjectionMatrix = CreateProjectionMatrixD3D(Width, Height);

	Windows.push_back(window);
}

void D3DWindowRender::RenderWindows()
{
	for (SharedPtr<UIWindow>& window : Windows)
	{
		SharedPtr<FWindowViewPort> viewport = window->GetViewport();

		if (viewport == nullptr) continue;

		GD3DDeviceContext->RSSetViewports(1, &viewport->ViewportInfo);

		ID3D11RenderTargetView* RTV = viewport->RenderTargetView;
		ID3D11DepthStencilView* DSV = viewport->DepthStencilView;

#if ALPHA_BLENDED_WINDOWS
		static float TransparentColor[4]{
			0.0f, 0.0f, 0.0f, 0.0f
		};

		if (window->GetTransparencySupport() == EWindowTransparency::PerPixel)
		{
			GD3DDeviceContext->ClearRenderTargetView(RTV, TransparentColor);
		}
#endif

		GD3DDeviceContext->OMSetRenderTargets(1, &RTV, NULL);



		GD3DDeviceContext->OMSetRenderTargets(0, NULL, NULL);


		const bool bUseVSync = false;
		viewport->D3DSwapChain->Present(bUseVSync ? 1 : 0, 0);
	}
}

void D3DWindowRender::OnWindowDestroy(const SharedPtr<UIWindow>& window)
{
	List_Remove(Windows, window);
}
