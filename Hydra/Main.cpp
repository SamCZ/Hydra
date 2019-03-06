#if 1

#include <iostream>

#include "Hydra/Render/Pipeline/DeviceManager11.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI_D3D11.h"
#include "Hydra/Render/Pipeline/BindingHelpers.h"
#include "Hydra/Render/Graphics.h"
#include "Hydra/Engine.h"

#include "Hydra/Scene/Spatial.h"

#include "Hydra/Render/RenderManager.h"
#include "Hydra/Render/RenderStageDeffered.h"
#include "Hydra/Render/Mesh.h"

#include "Hydra/Import/MeshImporter.h"
#include "Hydra/Scene/Components/Renderer.h"
#include "Hydra/Scene/Components/Camera.h"
#include "Hydra/Scene/Components/Movement/FirstPersonController.h"
#include "Hydra/Scene/Components/LodGroup.h"

#include "Hydra/Input/Windows/WindowsInputManager.h"

#include "Hydra/Core/FastNoise.h"
#include "Hydra/Core/Random.h"
#include "Hydra/Terrain/Erosion.h"

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
	WindowsInputManagerPtr _InputManager;

	RenderManagerPtr rm;
	RenderStageDefferedPtr rsd;

	inline LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		_InputManager->MsgProc(hWnd, uMsg, wParam, lParam);

		return S_OK;
	}

	inline void Escape()
	{
		_InputManager->ToggleMouseCapture();
	}

	inline float* GenerateNoiseMap(int mapSize)
	{
		int seed = 0;
		bool randomizeSeed = true;

		int numOctaves = 7;
		float persistence = .5f;
		float lacunarity = 2;
		float initialScale = 100.0f;

		auto map = new float[mapSize * mapSize];
		auto prng = Random(seed);

		Vector2* offsets = new Vector2[numOctaves];
		for (int i = 0; i < numOctaves; i++)
		{
			offsets[i] = Vector2(prng.GetFloat(-1000, 1000), prng.GetFloat(-1000, 1000));
		}

		float minValue = 999999.999f;
		float maxValue = -999999.999f;

		FastNoise noise;
		noise.SetNoiseType(FastNoise::NoiseType::Perlin);

		for (int y = 0; y < mapSize; y++)
		{
			for (int x = 0; x < mapSize; x++)
			{
				float noiseValue = 0;
				float scale = initialScale;
				float weight = 1;
				for (int i = 0; i < numOctaves; i++)
				{
					Vector2 p = offsets[i] + Vector2(x / (float)mapSize, y / (float)mapSize) * scale;
					noiseValue += noise.GetNoise(p.x, p.y) * weight;
					weight *= persistence;
					scale *= lacunarity;
				}
				map[y * mapSize + x] = noiseValue;
				minValue = std::min(noiseValue, minValue);
				maxValue = std::max(noiseValue, maxValue);
			}
		}

		delete[] offsets;

		// Normalize
		if (maxValue != minValue)
		{
			for (int i = 0; i < mapSize * mapSize; i++)
			{
				map[i] = (map[i] - minValue) / (maxValue - minValue);
			}
		}

		return map;
	}

	inline Mesh* GenerateTerrain()
	{
		int mapSize = 512;
		float scale = 20;
		float elevationScale = 10;
		int numErosionIterations = 50000 * 3;

		float* noise = GenerateNoiseMap(mapSize);

		Erosion erosion;
		erosion.Erode(noise, mapSize, numErosionIterations);

		List<Vector3> vertices;
		vertices.reserve(mapSize * mapSize);
		List<unsigned int> indices;
		indices.reserve((mapSize - 1) * (mapSize - 1) * 6);

		Log("GenerateTerrain", "Generating...");

		int t = 0;
		for (int y = 0; y < mapSize; y++)
		{
			for (int x = 0; x < mapSize; x++)
			{
				int i = y * mapSize + x;

				Vector2 percent = Vector2(x / (mapSize - 1.0f), y / (mapSize - 1.0f));
				Vector3 pos = Vector3(percent.x * 2 - 1, 0, percent.y * 2 - 1) * scale;
				pos += Vector3(0, 1, 0) * noise[i] * elevationScale;
				vertices.insert(vertices.begin() + i, pos);

				// Construct triangles
				if (x != mapSize - 1 && y != mapSize - 1)
				{
					indices.insert(indices.begin() + t + 0, i + mapSize);
					indices.insert(indices.begin() + t + 1, i + mapSize + 1);
					indices.insert(indices.begin() + t + 2, i);

					indices.insert(indices.begin() + t + 3, i + mapSize + 1);
					indices.insert(indices.begin() + t + 4, i + 1);
					indices.insert(indices.begin() + t + 5, i);

					t += 6;
				}
			}
		}

		Log("GenerateTerrain", "Done...");

		Mesh* mesh = new Mesh();
		mesh->Vertices = vertices;
		mesh->Indices = indices;

		mesh->GenerateNormals();

		Log("GenerateTerrain", "yo...");

		return mesh;
	}

	inline HRESULT DeviceCreated() override
	{
		Log("MainRenderView::DeviceCreated");

		_InputManager = MakeShared<WindowsInputManager>();

		_InputManager->AddAxisMapping("MoveForwardBackward", Keys::W, 1.0f);
		_InputManager->AddAxisMapping("MoveForwardBackward", Keys::S, -1.0f);

		_InputManager->AddAxisMapping("MoveLeftRight", Keys::A, 1.0f);
		_InputManager->AddAxisMapping("MoveLeftRight", Keys::D, -1.0f);

		_InputManager->AddAxisMapping("LookUpDown", Keys::MouseY, -0.1f);
		_InputManager->AddAxisMapping("LookLeftRight", Keys::MouseX, -0.1f);

		_InputManager->AddActionMapping("Esc", Keys::Escape);
		_InputManager->BindAction("Esc", IE_Pressed, this, &MainRenderView::Escape);

		Engine::SetInputManager(_InputManager);

		//_InputManager->SetMouseCapture(true);

		_renderInterface = MakeShared<NVRHI::RendererInterfaceD3D11>(&g_ErrorCallback, _deviceManager->GetImmediateContext());

		Engine::SetRenderInterface(_renderInterface);
		Engine::SetDeviceManager(_deviceManager);

		Graphics::Create();

		rm = MakeShared<RenderManager>();
		rm->MainScene = MakeShared<Spatial>("Main");

		SpatialPtr cameraObj = MakeShared<Spatial>("Camera");
		cameraObj->Position.y = 2;
		cameraObj->Position.z = 5;
		cameraObj->AddComponent<Camera>();
		cameraObj->AddComponent<FirstPersonController>();
		rm->MainScene->AddChild(cameraObj);

		/*SpatialPtr testModel = Meshimporter::Import("Assets/Sponza/SponzaNoFlag.obj", MeshImportOptions());
		testModel->Scale = Vector3(0.01f, 0.01f, 0.01f);
		testModel->AddComponent<LodGroup>();
		testModel->SetStatic(true);
		rm->MainScene->AddChild(testModel);*/

		/*SpatialPtr testModel = New(Spatial);
		RendererPtr voxelRender = testModel->AddComponent<Renderer>();
		voxelRender->TestColor = MakeRGB(200, 200, 200).toVec3();
		Mesh* mesh2 = GenerateTerrain();
		voxelRender->SetMesh(mesh2);
		rm->MainScene->AddChild(testModel);*/


		SpatialPtr box = MakeShared<Spatial>();
		RendererPtr r = box->AddComponent<Renderer>();
		r->SetMesh(Mesh::CreatePrimitive(PrimitiveType::Box));
		rm->MainScene->AddChild(box);

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
		_InputManager->Update();
		rm->MainScene->Update();

		//std::cout << Engine::GetDeviceManager()->GetAverageFrameTime() << std::endl;
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
#endif