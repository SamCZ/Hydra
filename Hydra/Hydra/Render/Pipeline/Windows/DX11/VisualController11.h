#pragma once

#include "DeviceManager11.h"
#include "GFSDK_NVRHI_D3D11.h"

#include "Hydra/Input/Windows/WindowsInputManager.h"
#include "UIRendererDX11.h"

namespace Hydra
{
	class HYDRA_API VisualController : public IVisualControllerDX11
	{
	public:
		VisualController(IVisualController* view) : IVisualControllerDX11(view) {}

		NVRHI::RendererInterfaceD3D11* RenderInterface;
		WindowsInputManager* InputManager;
		uint8 CreatedInputManager : 1;

		UIRendererDX11* UIRenderer;
		uint8 CreatedUIRenderer : 1;

		LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void Render(RenderTargetView RTV);
		void Animate(double fElapsedTimeSeconds);
		HRESULT DeviceCreated();
		void DeviceDestroyed();
		void BackBufferResized(uint32_t width, uint32_t height, uint32_t sampleCount);
	};
}