#pragma once

#include "Hydra/Core/Vector.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/SmartPointer.h"
#include "Hydra/App/Render/WindowRender.h"

#include <Windows.h>
#include <DXGI.h>
#include <D3D11.h>

class UIWindow;

struct FWindowViewPort
{
	Matrix4 ProjectionMatrix;
	D3D11_VIEWPORT ViewportInfo;
	IDXGISwapChain* D3DSwapChain;
	ID3D11Texture2D* BackBufferTexture;
	ID3D11RenderTargetView* RenderTargetView;
	ID3D11DepthStencilView* DepthStencilView;
	bool bFullscreen;

	FWindowViewPort() : DepthStencilView(NULL), bFullscreen(false)
	{

	}

	~FWindowViewPort()
	{
		D3DSwapChain->Release();
		BackBufferTexture->Release();
		RenderTargetView->Release();

		if (DepthStencilView)
		{
			DepthStencilView->Release();
		}
	}
};

class D3DWindowRender : public FWindowRender
{
private:
	ID3D11Device* GD3DDevice;
	ID3D11DeviceContext* GD3DDeviceContext;

	List<SharedPtr<UIWindow>> Windows;
public:

	D3DWindowRender();
	~D3DWindowRender();

	virtual void CreateViewport(SharedPtr<UIWindow>& window) override;

	virtual void RenderWindows() override;

	virtual void OnWindowDestroy(const SharedPtr<UIWindow>& window) override;

private:
	bool CreateDevice();

	void CreateBackBufferResources(IDXGISwapChain* InSwapChain, ID3D11Texture2D*& OutBackBuffer, ID3D11RenderTargetView*& OutRTV);
};