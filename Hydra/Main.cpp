#include <iostream>

#include "Hydra/Render/Pipeline/DeviceManager11.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI_D3D11.h"
#include "Hydra/Render/Pipeline/BindingHelpers.h"
#include "Hydra/Render/Graphics.h"
#include "Hydra/Engine.h"

#include "Hydra/Scene/Spatial.h"

#include "Hydra/Render/RenderManager.h"
#include "Hydra/Render/RenderStageDeffered.h"

#include "Hydra/Import/MeshImporter.h"
#include "Hydra/Scene/Components/Camera.h"

void signalError(const char* file, int line, const char* errorDesc)
{
	char buffer[4096];
	int length = (int)strlen(errorDesc);
	length = std::min<int>(length, 4000); // avoid a "buffer too small" exception for really long error messages
	sprintf_s(buffer, "%s:%i\n%.*s", file, line, length, errorDesc);

	OutputDebugStringA(buffer);
	OutputDebugStringA("\n");
	MessageBoxA(NULL, buffer, "ERROR", MB_ICONERROR | MB_OK);
}
#define CHECK_ERROR(expr, msg) if (!(expr)) signalError(__FILE__, __LINE__, msg)

class RendererErrorCallback : public NVRHI::IErrorCallback
{
	void signalError(const char* file, int line, const char* errorDesc)
	{
		char buffer[4096];
		int length = (int)strlen(errorDesc);
		length = std::min<int>(length, 4000); // avoid a "buffer too small" exception for really long error messages
		sprintf_s(buffer, "%s:%i\n%.*s", file, line, length, errorDesc);

		OutputDebugStringA(buffer);
		OutputDebugStringA("\n");
		MessageBoxA(NULL, buffer, "ERROR", MB_ICONERROR | MB_OK);
	}
};
RendererErrorCallback g_ErrorCallback;

using namespace Hydra;

static SharedPtr<DeviceManager> _deviceManager;

class MainRenderView : public IVisualController
{
private:
	SharedPtr<NVRHI::RendererInterfaceD3D11> _renderInterface;
public:
	RenderManagerPtr rm;
	RenderStageDefferedPtr rsd;

	inline HRESULT DeviceCreated() override
	{
		Log("MainRenderView::DeviceCreated");

		_renderInterface = MakeShared<NVRHI::RendererInterfaceD3D11>(&g_ErrorCallback, _deviceManager->GetImmediateContext());

		Engine::SetRenderInterface(_renderInterface);
		Engine::SetDeviceManager(_deviceManager);

		Graphics::Create();

		rm = MakeShared<RenderManager>();
		rm->MainScene = MakeShared<Spatial>("Main");

		SpatialPtr cameraObj = MakeShared<Spatial>("Camera");
		cameraObj->Position.y = 2;
		cameraObj->Position.z = 5;
		CameraPtr cam = cameraObj->AddComponent<Camera>();
		rm->MainScene->AddChild(cameraObj);

		SpatialPtr testModel = Meshimporter::Import("Assets/IndustryEmpire/Models/BrickFactory.fbx", MeshImportOptions());
		testModel->Scale = Vector3(0.01f, 0.01f, 0.01f);
		rm->MainScene->AddChild(testModel);

		rsd = MakeShared<RenderStageDeffered>();

		rm->MainScene->Start();

		return S_OK;
	}

	inline void BackBufferResized(uint32_t width, uint32_t height, uint32_t sampleCount) override
	{
		Log("MainRenderView::BackBufferResized", ToString(width) + ", " + ToString(height) + ", " + ToString(sampleCount), "DeviceCreated");

		Engine::ScreenSize = Vector2(width, height);

		for (CameraPtr camera : Camera::AllCameras)
		{
			camera->Resize(width, height);
		}

		rsd->AllocateViewDependentResources(width, height, sampleCount);
	}

	inline void DeviceDestroyed() override
	{
		Log("MainRenderView::DeviceDestroyed");

		Graphics::Destroy();
	}

	void Render(RenderTargetView RTV) override
	{
		ID3D11Resource* pMainResource = NULL;
		RTV->GetResource(&pMainResource);
		NVRHI::TextureHandle mainRenderTarget = _renderInterface->getHandleForTexture(pMainResource);
		pMainResource->Release();

		rsd->Render(rm);

		Graphics::Blit(rsd->GetOutputName(), mainRenderTarget);

		_renderInterface->forgetAboutTexture(pMainResource);
	}

	void Animate(double fElapsedTimeSeconds) override
	{
		rm->MainScene->Update();
	}
};

int main()
{
	_deviceManager = MakeShared<DeviceManager>();

	DeviceCreationParameters deviceParams;
	deviceParams.backBufferWidth = 1280;
	deviceParams.backBufferHeight = 720;
	deviceParams.swapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	deviceParams.swapChainSampleCount = 1;
	deviceParams.swapChainBufferCount = 4;
	deviceParams.startFullscreen = false;
	deviceParams.startMaximized = false;
	deviceParams.enableDebugRuntime = false;
	deviceParams.refreshRate = 120;

	Engine::ScreenSize = Vector2(deviceParams.backBufferWidth, deviceParams.backBufferHeight);

	MainRenderView mainRenderView;
	_deviceManager->AddControllerToFront(&mainRenderView);

	//GuiVisualController guiView(_deviceManager);
	//_deviceManager->AddControllerToFront(&guiView);


	std::string title = "Hydra | DX11";

	wchar_t wchTitle[256];
	MultiByteToWideChar(CP_ACP, 0, title.c_str(), -1, wchTitle, 256);

	if (FAILED(_deviceManager->CreateWindowDeviceAndSwapChain(deviceParams, wchTitle)))
	{
		MessageBox(NULL, L"Cannot initialize the DirextX11 device with the requested parameters", L"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	_deviceManager->MessageLoop();
	_deviceManager->Shutdown();

	return 0;
}