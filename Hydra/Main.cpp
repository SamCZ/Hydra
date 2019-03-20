#if 1

#include <iostream>

#include "Hydra/Render/Pipeline/DeviceManager11.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI_D3D11.h"
#include "Hydra/Render/Pipeline/BindingHelpers.h"
#include "Hydra/Render/Graphics.h"
#include "Hydra/Engine.h"

#include "Hydra/Scene/Spatial.h"

#include "Hydra/Render/RenderManager.h"
#include "Hydra/Render/RenderStage/RenderStageDeffered.h"
#include "Hydra/Render/Mesh.h"
#include "Hydra/Render/TextureLayoutDef.h"
#include "Hydra/Render/Material.h"

#include "Hydra/Import/MeshImporter.h"
#include "Hydra/Import/TextureImporter.h"

#include "Hydra/Scene/Components/Renderer.h"
#include "Hydra/Scene/Components/Camera.h"
#include "Hydra/Scene/Components/Movement/FirstPersonController.h"
#include "Hydra/Scene/Components/LodGroup.h"
#include "Hydra/Scene/Components/Light.h"

#include "Hydra/Input/Windows/WindowsInputManager.h"

#include "Hydra/Core/FastNoise.h"
#include "Hydra/Core/Random.h"
#include "Hydra/Terrain/Erosion.h"

#include "Hydra/Render/Pipeline/DX11/UIRendererDX11.h"

#include "ImGui/imgui.h"
#include "ImGui/ImGuizmo.h"
//#include "ImGui/imgui_helper.h"

#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_style.h"

#include <functional>

#include "Hydra/Render/TestVars.h"

#include "Hydra/Terrain/Terrain.h"



/* Set the better graphic card for notebooks ( ͡° ͜ʖ ͡°)
*  http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
*  http://stevendebock.blogspot.com/2013/07/nvidia-optimus.html
*/
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

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

static SharedPtr<DeviceManager> _deviceManager = nullptr;
static SharedPtr<NVRHI::RendererInterfaceD3D11> _renderInterface = nullptr;
static SpatialPtr lightObj = nullptr;


class UIRenderView : public IVisualController
{
private:
	NVGcontext* _Context = nullptr;
	UIRendererDX11* _UIRenderer;
public:
	inline HRESULT DeviceCreated() override
	{
		Log("UIRenderView::DeviceCreated");

		if (_renderInterface == nullptr)
		{
			_renderInterface = MakeShared<NVRHI::RendererInterfaceD3D11>(&g_ErrorCallback, _deviceManager->GetImmediateContext());
		}

		_UIRenderer = new UIRendererDX11(_deviceManager->GetDevice());
		_UIRenderer->Create();



		return S_OK;
	}

	inline void DeviceDestroyed() override
	{
		_UIRenderer->Destroy();
		delete _UIRenderer;
	}

	void Render(RenderTargetView RTV) override
	{
		ID3D11Resource* pMainResource = NULL;
		RTV->GetResource(&pMainResource);
		NVRHI::TextureHandle mainRenderTarget = _renderInterface->getHandleForTexture(pMainResource);
		pMainResource->Release();

		_UIRenderer->Begin();

		_UIRenderer->DrawImage(Graphics::GetRenderTarget("DirLight_ShadowMa_ColorTest"), 0, 0, 100, 100, 0.0f);

		_UIRenderer->End();

		_renderInterface->forgetAboutTexture(pMainResource);
	}

	inline void BackBufferResized(uint32_t width, uint32_t height, uint32_t sampleCount) override
	{
		Engine::ScreenSize = Vector2(width, height);
	}
};

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class ImGuiRenderView : public IVisualController
{
private:

public:
	inline HRESULT DeviceCreated() override
	{
		ImGui_ImplDX11_Init(_deviceManager->GetHWND(), _deviceManager->GetDevice(), _deviceManager->GetImmediateContext());

		ImGui::StyleColorsClassic();
		ImGui_CustomStyle();
		return S_OK;
	}

	inline void DeviceDestroyed() override
	{
		ImGui_ImplDX11_Shutdown();
	}

	virtual LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		return 1;
	}

	void Render(RenderTargetView RTV) override
	{
		(void)RTV;
		ImGui_ImplDX11_NewFrame();

		ImGui::DragFloat("Left", &Test::OR_LEFT, 0.1f);
		ImGui::DragFloat("Right", &Test::OR_RIGHT, 0.1f);
		ImGui::DragFloat("Bottom", &Test::OR_BOTTOM, 0.1f);
		ImGui::DragFloat("Top", &Test::OR_TOP, 0.1f);

		ImGui::DragFloat("Near", &Test::OR_NEAR, 0.1f);
		ImGui::DragFloat("Far", &Test::OR_FAR, 0.1f);

		ImGui::DragFloat("Bias", &lightObj->GetComponent<Light>()->DepthBias, 0.0001f);

		ImGui::Render();
	}

	inline void BackBufferResized(uint32_t width, uint32_t height, uint32_t sampleCount) override
	{
		Engine::ScreenSize = Vector2(width, height);
	}
};

class MainRenderView : public IVisualController
{
public:
	WindowsInputManagerPtr _InputManager;

	RenderManagerPtr rm;
	RenderStageDefferedPtr rsd;
	TextureLayoutDefPtr texLayout;

	

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

		List<VertexBufferEntry> vertices;
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

				VertexBufferEntry entry = {};
				entry.position = pos;

				vertices.insert(vertices.begin() + i, entry);

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
		mesh->VertexData = vertices;
		mesh->Indices = indices;

		mesh->GenerateNormals();

		mesh->UpdateBuffers();

		Log("GenerateTerrain", "yo...");

		return mesh;
	}

	inline void RecompileDefaultShader()
	{
		Log("RecompileDefaultShader");

		/*TechniquePtr tech = Graphics::GetTechnique("DefaultPBRShader");

		tech->SetKeywordByHash(tech->GetKeywordHash({ "USE_ALBEDO_TEX" }));
		tech->Recompile(true);

		Graphics::GetTechnique("SSAO")->Recompile(true);*/
	}

	inline void RotX(float val)
	{
		lightObj->Rotation.x += val * 0.1f;
	}

	inline void RotY(float val)
	{
		lightObj->Rotation.y += val * 0.1f;
	}

	inline void AddPointLight()
	{
		SpatialPtr pl = MakeShared<Spatial>("Light");
		pl->Position = Camera::MainCamera->GameObject->Position;

		LightPtr light = pl->AddComponent<Light>();

		light->Type = LightType::Point;
		light->Range = 2;
		light->Color = MakeRGB(20, 100, 200);

		rm->MainScene->AddChild(pl);
	}

	inline HRESULT DeviceCreated() override
	{
		Log("MainRenderView::DeviceCreated");

		_InputManager = MakeShared<WindowsInputManager>();

		_InputManager->AddAxisMapping("RotX", Keys::I, 1.0f);
		_InputManager->AddAxisMapping("RotX", Keys::K, -1.0f);

		_InputManager->AddAxisMapping("RotY", Keys::J, 1.0f);
		_InputManager->AddAxisMapping("RotY", Keys::L, -1.0f);


		_InputManager->AddAxisMapping("MoveForwardBackward", Keys::W, 1.0f);
		_InputManager->AddAxisMapping("MoveForwardBackward", Keys::S, -1.0f);
		_InputManager->AddAxisMapping("MoveLeftRight", Keys::A, 1.0f);
		_InputManager->AddAxisMapping("MoveLeftRight", Keys::D, -1.0f);

		_InputManager->AddAxisMapping("LookUpDown", Keys::MouseY, -0.1f);
		_InputManager->AddAxisMapping("LookLeftRight", Keys::MouseX, -0.1f);

		_InputManager->AddActionMapping("Esc", Keys::Escape);
		_InputManager->BindAction("Esc", IE_Pressed, this, &MainRenderView::Escape);

		_InputManager->AddActionMapping("F5", Keys::F5);
		_InputManager->BindAction("F5", IE_Pressed, this, &MainRenderView::RecompileDefaultShader);

		_InputManager->BindAxis("RotX", this, &MainRenderView::RotX);
		_InputManager->BindAxis("RotY", this, &MainRenderView::RotY);


		_InputManager->AddActionMapping("Space", Keys::SpaceBar);
		_InputManager->BindAction("Space", IE_Pressed, this, &MainRenderView::AddPointLight);

		Engine::SetInputManager(_InputManager);

		//_InputManager->SetMouseCapture(true);

		if (_renderInterface == nullptr)
		{
			_renderInterface = MakeShared<NVRHI::RendererInterfaceD3D11>(&g_ErrorCallback, _deviceManager->GetImmediateContext());
		}

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

		{
			lightObj = MakeShared<Spatial>("Light");
			//lightObj->Position = Vector3(-50.0f, 1300.0f, -40.0f);
			lightObj->Position = Vector3(0, 10, 0);
			lightObj->Rotation = Vector3(-90, 0, 0.0f);
			LightPtr light = lightObj->AddComponent<Light>();
			light->ShadowType = ShadowType::Soft;
			light->Intensity = 1.0f;
			rm->MainScene->AddChild(lightObj);
		}

		SpatialPtr testModel = Meshimporter::Import("Assets/Sponza/SponzaNoFlag.obj", MeshImportOptions());
		//SpatialPtr testModel = Meshimporter::Import("Assets/IndustryEmpire/Models/BrickFactory.fbx", MeshImportOptions());
		testModel->Scale = Vector3(0.01f, 0.01f, 0.01f);
		//testModel->AddComponent<LodGroup>();
		testModel->SetStatic(true);
		rm->MainScene->AddChild(testModel);



		/*SpatialPtr testModel = New(Spatial);
		RendererPtr voxelRender = testModel->AddComponent<Renderer>();
		voxelRender->TestColor = MakeRGB(200, 200, 200).toVec3();
		Mesh* mesh2 = GenerateTerrain();
		voxelRender->SetMesh(mesh2);
		rm->MainScene->AddChild(testModel);*/


		/*SpatialPtr box = MakeShared<Spatial>();
		RendererPtr r = box->AddComponent<Renderer>();
		r->SetMesh(Mesh::CreatePrimitive(PrimitiveType::Box));
		r->Mat.Albedo = TextureImporter::Import("Assets/Textures/Rock2.dds");
		rm->MainScene->AddChild(box);

		class TestRotationComponent : public Component
		{
		public:
			inline void Start()
			{

			}
			inline void Update()
			{
				Parent->Rotation += 0.1f;

				Parent->Scale = Vector3(cos(Parent->Rotation.x * 0.1f) + 0.5f, sin(Parent->Rotation.y * 0.1f) + 0.5f, cos(Parent->Rotation.z * 0.1f) + 0.5f);
			}
		};
		box->AddComponent<TestRotationComponent>();*/

		texLayout = MakeShared<TextureLayoutDef>();

		rsd = MakeShared<RenderStageDeffered>();
		rsd->SetTextureLayoutDef(texLayout);

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

		Graphics::AllocateViewDependentResources(width, height, sampleCount);
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

	UIRenderView uiRenderView;
	_deviceManager->AddControllerToFront(&uiRenderView);

	ImGuiRenderView imGuiRenderView;
	_deviceManager->AddControllerToFront(&imGuiRenderView);

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