#include "VisualController11.h"
#include "Hydra/Render/Graphics.h"
#include "Hydra/EngineContext.h"

namespace Hydra
{
	LRESULT VisualController::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (CreatedInputManager)
		{
			return InputManager->MsgProc(hWnd, uMsg, wParam, lParam);
		}

		return S_OK;
	}

	void VisualController::Render(RenderTargetView RTV)
	{
		ID3D11Resource* pMainResource = NULL;
		RTV->GetResource(&pMainResource);
		NVRHI::TextureHandle mainRenderTarget = RenderInterface->getHandleForTexture(pMainResource);
		pMainResource->Release();

		if (CreatedUIRenderer)
		{
			UIRenderer->Begin(Context->ScreenSize);
		}

		_EngineVisualController->OnRender(mainRenderTarget);

		if (CreatedUIRenderer)
		{
			UIRenderer->End();
		}

		RenderInterface->forgetAboutTexture(pMainResource);
	}

	void VisualController::Animate(double fElapsedTimeSeconds)
	{
		if (CreatedInputManager)
		{
			InputManager->Update();
		}

		_EngineVisualController->OnTick((float)fElapsedTimeSeconds);
	}

	HRESULT VisualController::DeviceCreated()
	{
		if (Context->GetRenderInterface() == nullptr)
		{
			Context->SetRenderInterface(DeviceManager::CreateRenderInterfaceForPlatform(Context->GetDeviceManager()));
		}

		RenderInterface = static_cast<NVRHI::RendererInterfaceD3D11*>(Context->GetRenderInterface());

		if (Context->GetGraphics() == nullptr)
		{
			Context->SetGraphics(new Graphics(Context));
		}

		if (Context->GetInputManager() == nullptr)
		{
			CreatedInputManager = true;

			InputManager = new WindowsInputManager(Context);
			Context->SetInputManager(InputManager);
		}

		if (Context->GetUIRenderer() == nullptr && !CreatedInputManager)
		{
			UIRenderer = new UIRendererDX11(Context);
			UIRenderer->Create();
			Context->SetUIRenderer(UIRenderer);
			CreatedUIRenderer = true;
		}

		_EngineVisualController->OnCreated();

		return S_OK;
	}

	void VisualController::DeviceDestroyed()
	{
		if (CreatedUIRenderer)
		{
			UIRenderer->Destroy();
			delete UIRenderer;
		}

		if (Context->GetRenderInterface() != nullptr)
		{
			delete Context->GetRenderInterface();
			Context->SetRenderInterface(nullptr);
		}

		if (Context->GetGraphics() != nullptr)
		{
			delete Context->GetGraphics();
			Context->SetGraphics(nullptr);
		}

		if (Context->GetInputManager() != nullptr)
		{
			delete Context->GetInputManager();
			Context->SetInputManager(nullptr);
		}

		_EngineVisualController->OnDestroy();

		delete _EngineVisualController;
	}

	void VisualController::BackBufferResized(uint32_t width, uint32_t height, uint32_t sampleCount)
	{
		_EngineVisualController->OnResize(width, height, sampleCount);
	}
}