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

#include "Hydra/Render/Pipeline/DX11/UIRendererDX11.h"

#include "ImGui/imgui.h"
#include "ImGui/ImGuizmo.h"
#include "ImGui/imgui_helper.h"

#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_style.h"

#include <functional>

#include "Hydra/Render/TestVars.h"

#include "Hydra/Terrain/Generator/Noise/NoiseMap.h"
#include "Hydra/Terrain/Generator/MeshGenerator.h"

#include "Hydra/Terrain/Marching/MarchingCubesTable.h"

#include "Hydra/Core/Random.h"

#include "Hydra/Physics/Collisons/Testing.h"

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

static TexturePtr uavTex;
static TexturePtr heigthMap;

static float paintRadius = 10.0f;

struct CollisionBuffer
{
	int Result;
	Vector3 V0;
	Vector3 V1;
	Vector3 V2;
	Vector3 Normal;
};

static CollisionBuffer storeColl = {};

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

		//_UIRenderer->DrawImage(Graphics::GetRenderTarget("DirLight_ShadowMa_ColorTest"), 0, 0, 100, 100, 0.0f);

		//_UIRenderer->DrawImage(uavTex, 0, 105, 100, 100, 0.0f);

		//_UIRenderer->DrawImage(heigthMap, 105, 105, 100, 100, 0.0f);

		_UIRenderer->DrawImage(uavTex, 0, 0, heigthMap->GetDesc().width, heigthMap->GetDesc().height, 0.0f);
		
		Vector2i mPos = Engine::GetInputManager()->GetCursorPos();

		if (mPos.x <= heigthMap->GetDesc().width && mPos.y <= heigthMap->GetDesc().height)
		{
			_UIRenderer->DrawOval(mPos.x, mPos.y, paintRadius * 2.0f, 0, MakeRGB(255, 0, 0));
		}

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

		BeginIds();

		ImGui::Begin("Debug##Default");
		ImGui::DragFloat("Left", &Test::OR_LEFT, 0.1f);
		ImGui::DragFloat("Right", &Test::OR_RIGHT, 0.1f);
		ImGui::DragFloat("Bottom", &Test::OR_BOTTOM, 0.1f);
		ImGui::DragFloat("Top", &Test::OR_TOP, 0.1f);

		ImGui::DragFloat("Near", &Test::OR_NEAR, 0.1f);
		ImGui::DragFloat("Far", &Test::OR_FAR, 0.1f);

		ImGui::DragFloat("Bias", &lightObj->GetComponent<Light>()->DepthBias, 0.0001f);
		ImGui::End();

		ImGui::Begin("Material params");
		ITER(Material::AllMaterials, it1)
		{
			MaterialPtr mat = it1->second;

			if (mat->IsInternalMaterial) continue;

			if (CollapsingHeader(it1->first))
			{
				for (auto& kv : mat->GetVarTypes())
				{
					Var* var = mat->GetRawVar(kv.first);

					switch (kv.second)
					{
						case VarType::Float:
						{
							if (var == nullptr)
							{
								mat->SetFloat(kv.first, 1.0f);
								var = mat->GetRawVar(kv.first);
							}

							if (DragFloat(kv.first, (float*)mat->GetRawVarData(kv.first), 0.01f))
							{
								var->HasChnaged = true;
							}
						}
						break;

						case VarType::Vector2:
						{
							if (var == nullptr)
							{
								mat->SetVector2(kv.first, Vector2(0));
								var = mat->GetRawVar(kv.first);
							}

							if (DragFloat2(kv.first, (float*)mat->GetRawVarData(kv.first), 0.01f))
							{
								var->HasChnaged = true;
							}
						}
						break;
						

						case VarType::Vector3:
						{
							if (var == nullptr)
							{
								mat->SetVector3(kv.first, Vector3(0));
								var = mat->GetRawVar(kv.first);
							}

							if (DragFloat3(kv.first, (float*)mat->GetRawVarData(kv.first), 0.01f))
							{
								var->HasChnaged = true;
							}
						}
						break;

						case VarType::Vector4:
						{
							if (var == nullptr)
							{
								mat->SetVector4(kv.first, Vector4(0));
								var = mat->GetRawVar(kv.first);
							}

							if (DragFloat4(kv.first, (float*)mat->GetRawVarData(kv.first), 0.01f))
							{
								var->HasChnaged = true;
							}
						}
						break;
					}
				}
			}
		}
		ImGui::End();


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

	MaterialPtr _SkyMaterial;

	SpatialPtr box;

	inline LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		_InputManager->MsgProc(hWnd, uMsg, wParam, lParam);

		return S_OK;
	}

	inline void Escape()
	{
		_InputManager->ToggleMouseCapture();
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

	bool isHoldingMouse;

	inline void MouseDown()
	{
		isHoldingMouse = true;
		Paint(Vector2(_InputManager->GetCursorPos()));

		//storeColl

		MaterialPtr paint3d = Material::CreateOrGet("Paint3D", (File)"Assets/Shaders/Utils/GPU/Paint3D.hlsl");
		paint3d->SetVector3("_ArrPos", storeColl.V1);
		Graphics::Dispatch(paint3d, 1, 1, 1);

		MaterialPtr clearMat = Material::CreateOrGet("Clear3D", (File)"Assets/Shaders/Utils/GPU/ClearProceduralMesh.hlsl");
		Graphics::Dispatch(clearMat, 64 / 8, 64 / 8, 64 / 8);

		MaterialPtr normalGen = Material::CreateOrGet("nmpf3d", (File)"Assets/Shaders/Utils/GPU/NormalMapFrom3DNoise.hlsl");
		Graphics::Dispatch(normalGen, 64 / 8, 64 / 8, 64 / 8);

		MaterialPtr mcm = Material::CreateOrGet("amsflkas", (File)"Assets/Shaders/Utils/GPU/MarchingCubes.hlsl");
		Graphics::Dispatch(mcm, 64 / 8, 64 / 8, 64 / 8);
	}

	inline void MouseUp()
	{
		isHoldingMouse = false;
	}

	inline void MouseMoveXY(float val)
	{
		if (isHoldingMouse)
		{
			Paint(_InputManager->GetCursorPos());
		}
	}

	inline void OnMouseWheel(float val)
	{
		paintRadius += val;
	}

	inline void Paint(Vector2 pos)
	{
		MaterialPtr paintMat = Material::CreateOrGet("Painter", (File)"Assets/Shaders/Utils/GPU/Paint.hlsl");
		paintMat->SetTexture("_HeightMap", heigthMap);
		paintMat->SetVector2("_Position", pos);
		paintMat->SetFloat("_Radius", paintRadius);
		Graphics::Dispatch(paintMat, 16, 16, 1);
	}

	inline HRESULT DeviceCreated() override
	{
		Log("MainRenderView::DeviceCreated");

		_InputManager = MakeShared<WindowsInputManager>();

		_InputManager->AddActionMapping("MouseX", Keys::LeftMouseButton);
		_InputManager->BindAction("MouseX", IE_Pressed, this, &MainRenderView::MouseDown);
		_InputManager->BindAction("MouseX", IE_Released, this, &MainRenderView::MouseUp);


		_InputManager->AddAxisMapping("MouseWheel", Keys::MouseWheelAxis, 1.0f);
		_InputManager->BindAxis("MouseWheel", this, &MainRenderView::OnMouseWheel);


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

		_InputManager->BindAxis("LookUpDown", this, &MainRenderView::MouseMoveXY);
		_InputManager->BindAxis("LookLeftRight", this, &MainRenderView::MouseMoveXY);

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
			light->Intensity = 2.0f;
			rm->MainScene->AddChild(lightObj);
		}
		 
		/*VoxelTerrainPtr terrain = MakeShared<VoxelTerrain>();
		rm->MainScene->AddChild(terrain);*/

		struct Voxel
		{
			Vector4 Position;
		};
		
		Voxel* voxels = new Voxel[216];

		for (int x = 0; x < 6; x++)
		{
			for (int z = 0; z < 6; z++)
			{
				for (int y = 0; y < 6; y++)
				{
					float val = (cos(x) * sin(z)) + 0.5f;

					if (y > 3 && cos(x * z) > 0.0f) val = 0;

					voxels[x + y * 6 + z * 6 * 6].Position = Vector4(x, y, z, val);
				}
			}
		}

		_SkyMaterial = Material::CreateOrGet("Assets/Shaders/Sky.hlsl", true, false);
		_SkyMaterial->SetVector3("_SpherePos", Vector3(1.5, 0, 0));
		_SkyMaterial->SetFloat("_MergeWeight", 1.0);
		_SkyMaterial->SetStructArray("_Voxels", voxels, 216 * sizeof(Voxel));

		MaterialPtr terrainMat = Material::CreateOrGet("Assets/Shaders/VoxelTerrain.hlsl");
		terrainMat->SetTexture("_GrassTex", TextureImporter::Import("Assets/IndustryEmpire/Textures/TilePatine_D.TGA"));
		terrainMat->SetTexture("_GrassNormalTex", TextureImporter::Import("Assets/IndustryEmpire/Textures/TilePatine_N.TGA"));
		terrainMat->SetSampler("DefaultSampler", Graphics::CreateSampler("TerrainSampler"));

		terrainMat->SetTexture("_LayerTex0", TextureImporter::Import("Assets/Textures/Terrain/Grass.png"));
		terrainMat->SetTexture("_LayerTex1", TextureImporter::Import("Assets/Textures/Terrain/Rocks 1.png"));
		terrainMat->SetTexture("_LayerTex2", TextureImporter::Import("Assets/Textures/Terrain/Rocks 2.png"));
		terrainMat->SetTexture("_LayerTex3", TextureImporter::Import("Assets/Textures/Terrain/Sandy grass.png"));
		terrainMat->SetTexture("_LayerTex4", TextureImporter::Import("Assets/Textures/Terrain/Snow.png"));
		terrainMat->SetTexture("_LayerTex5", TextureImporter::Import("Assets/Textures/Terrain/Stony ground.png"));
		terrainMat->SetTexture("_LayerTex6", TextureImporter::Import("Assets/Textures/Terrain/Water.png"));

		MeshSettings meshSettings = {};
		meshSettings.UseFlatShading = false;
		meshSettings.MeshScale = 1.0f;
		meshSettings.ChunkSizeIndex = 8;
		meshSettings.UseGPUTexturing = true;

		AddChunk(meshSettings, terrainMat, 0, 0, 0);

		CreateVoxelTerrainInGPU();

		/*for (int x = -5; x <= 5; x++)
		{
			for (int y = -5; y <= 5; y++)
			{
				int distance = (int)glm::abs(glm::distance(Vector2(x, y), Vector2(0, 0)));
				if (distance > 4) distance = 4;

				AddChunk(meshSettings, terrainMat, meshSettings.GetMeshWorldSize() * x, meshSettings.GetMeshWorldSize() * y, distance);
			}
		}*/

		box = MakeShared<Spatial>();
		RendererPtr r = box->AddComponent<Renderer>();
		r->SetMesh(Mesh::CreatePrimitive(PrimitiveType::Box));
		rm->MainScene->AddChild(box);

		/*class TestRotationComponent : public Component
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

	struct VoxelBuffer
	{
		Vector4 Position;
		Vector3 Normal;
	};

	inline NVRHI::BufferHandle CreateBuffer(int size, void* data, bool readOnly = true)
	{
		NVRHI::BufferDesc bufferDesc;
		bufferDesc.byteSize = size;
		bufferDesc.canHaveUAVs = !readOnly;
		return Engine::GetRenderInterface()->createBuffer(bufferDesc, data);
	}
	
	inline void CreateVoxelTerrainInGPU()
	{
		const int N = 64;
		const int SIZE = N * N * N * 3 * 5;

		float* heightMap = new float[N * N * N];
		
		Random rnd;

		FastNoise noise;
		noise.SetNoiseType(FastNoise::NoiseType::Perlin);

		float scale = 5.0;

		for (int x = 0; x < N; x++)
		{
			for (int y = 0; y < N; y++)
			{
				for (int z = 0; z < N; z++)
				{
					heightMap[x + y * N + z * N * N] = (noise.GetNoise(x * scale, y * scale, z * scale) + 0.5f);
					//heightMap[x + y * N + z * N * N] = 0.0;
				}
			}
		}

		NVRHI::BufferDesc bufferDescMap;
		bufferDescMap.byteSize = N * N * N * sizeof(float);
		bufferDescMap.canHaveUAVs = true;
		NVRHI::BufferHandle heightMap3D = Engine::GetRenderInterface()->createBuffer(bufferDescMap, heightMap);
		delete[] heightMap;

		/*MaterialPtr genMapMat = Material::CreateOrGet("Assets/Shaders/Utils/GPU/GenerateMap.hlsl");
		genMapMat->SetBuffer("_VoxelMap", heightMap3D);	
		Graphics::Dispatch(genMapMat, N / 8, N / 8, N / 8);*/
		
		TexturePtr normalMap = Graphics::CreateUAVTexture3D("3DHeightmap", NVRHI::Format::RGBA32_FLOAT, N, N, N);

		MaterialPtr normalGen = Material::CreateOrGet("nmpf3d", (File)"Assets/Shaders/Utils/GPU/NormalMapFrom3DNoise.hlsl");
		normalGen->SetInt("_Width", N);
		normalGen->SetInt("_Height", N);
		normalGen->SetBuffer("_Noise", heightMap3D);
		normalGen->SetTexture("_Result", normalMap);
		Graphics::Dispatch(normalGen, N / 8, N / 8, N / 8);

		MaterialPtr paint3d = Material::CreateOrGet("Paint3D", (File)"Assets/Shaders/Utils/GPU/Paint3D.hlsl");
		paint3d->SetBuffer("_Voxels", heightMap3D);

		Vector4 pos = Vector4(-1);

		VoxelBuffer* EmptyData = new VoxelBuffer[SIZE];

		for (int i = 0; i < SIZE; i++)
		{
			EmptyData[i].Position = pos;
			EmptyData[i].Normal = pos;
		}

		NVRHI::BufferDesc bufferDesc;
		bufferDesc.byteSize = SIZE * sizeof(VoxelBuffer);
		bufferDesc.canHaveUAVs = true;
		NVRHI::BufferHandle buffer = Engine::GetRenderInterface()->createBuffer(bufferDesc, EmptyData);

		//delete[] EmptyData;

		MaterialPtr clearMat = Material::CreateOrGet("Clear3D", (File)"Assets/Shaders/Utils/GPU/ClearProceduralMesh.hlsl");
		clearMat->SetBuffer("_Buffer", buffer);
		clearMat->SetInt("_Width", N);
		clearMat->SetInt("_Height", N);
		clearMat->SetInt("_Depth", N);

		MaterialPtr mcm = Material::CreateOrGet("amsflkas", (File)"Assets/Shaders/Utils/GPU/MarchingCubes.hlsl");
		mcm->SetFloat("_Target", 0.5);
		mcm->SetInt("_Width", N);
		mcm->SetInt("_Height", N);
		mcm->SetInt("_Depth", N);
		mcm->SetInt("_Border", 1);

		mcm->SetTexture("_Normals", normalMap);

		mcm->SetBuffer("_Buffer", buffer);
		mcm->SetBuffer("_Voxels", heightMap3D);
		mcm->SetBuffer("_CubeEdgeFlags", CreateBuffer(256 * sizeof(int), MarchingCubesTable::CubeEdgeFlags));
		mcm->SetBuffer("_TriangleConnectionTable", CreateBuffer(256 * 16 * sizeof(int), MarchingCubesTable::TriangleConnectionTable));
		Graphics::Dispatch(mcm, N / 8, N / 8, N / 8);


		MaterialPtr mat = Material::CreateOrGet("Assets/Shaders/ProceduralDeffered.hlsl");
		mat->SetBuffer("_Buffer", buffer);


		CollisionBuffer emptyBuff = {};
		NVRHI::BufferDesc bufferDesc2;
		bufferDesc2.byteSize = sizeof(CollisionBuffer);
		bufferDesc2.canHaveUAVs = true;
		NVRHI::BufferHandle collHandle = Engine::GetRenderInterface()->createBuffer(bufferDesc2, &emptyBuff);

		MaterialPtr colMat = Material::CreateOrGet("PCollison", (File)"Assets/Shaders/Utils/GPU/ProceduralCollision.hlsl");
		colMat->SetBuffer("_Buffer", buffer);
		colMat->SetBuffer("_Collisions", collHandle);
		colMat->SetBuffer("_Voxels", heightMap3D);

		SpatialPtr sp = MakeShared<Spatial>();
		RendererPtr r = sp->AddComponent<Renderer>();

		Mesh* proceduralMesh = new Mesh();
		

		if (false) // Use complex collisions
		{
			size_t bufferSize = SIZE * sizeof(VoxelBuffer);
			Engine::GetRenderInterface()->readBuffer(buffer, EmptyData, &bufferSize);

			int index = 0;
			for (int i = 0; i < SIZE; i++)
			{
				VoxelBuffer& bff = EmptyData[i];

				if (bff.Position.w != -1)
				{
					VertexBufferEntry e = {};
					e.position.x = bff.Position.x;
					e.position.y = bff.Position.y;
					e.position.z = bff.Position.z;
					e.normal = bff.Normal;

					proceduralMesh->VertexData.emplace_back(e);
					proceduralMesh->Indices.push_back(index);

					index++;
				}
			}

			proceduralMesh->CreateComplexCollider();
		}

		proceduralMesh->SetVertexBuffer(buffer);
		proceduralMesh->SetIndexCount(SIZE);
		proceduralMesh->SetIndexed(false);
		r->Material = mat;

		r->SetMesh(proceduralMesh);
		rm->MainScene->AddChild(sp);
	}

	inline void AddChunk(const MeshSettings& meshSettings, MaterialPtr material, float x, float y, int levelOfDetail)
	{
		HeightMap* hMap = NoiseMap::GenerateHeightMap(meshSettings, Vector2(x, -y));
		hMap->InitalizeTexture(true);
		hMap->UploadTextureData();

		material->SetTexture("_HeightMap", hMap->Texture);

		int numVertsPerLine = meshSettings.GetNumVertsPerLine();
		int skipIncrement = (levelOfDetail == 0) ? 1 : levelOfDetail * 2;

		material->SetInt("g_numVertsPerLine", numVertsPerLine);
		material->SetInt("g_skipIncrement", skipIncrement);
		material->SetFloat("g_HeightScale", 50.0f);

		heigthMap = hMap->Texture;

		//Generate Normal map

		uavTex = Graphics::CreateUAVTexture("TestNormalMap", NVRHI::Format::RGBA16_FLOAT, hMap->Width - 2, hMap->Height - 2);

		MaterialPtr computeMat = Material::CreateOrGet("NormalMapCmp", (File)"Assets/Shaders/Utils/GPU/NormalMapFromHeightMap.hlsl");
		computeMat->SetTexture("_OutNormalMap", uavTex);
		computeMat->SetTexture("_HeightMap", hMap->Texture);
		computeMat->SetFloat("_Strength", 5.0f);

		Graphics::Dispatch(computeMat, hMap->Width / 16, hMap->Height / 16, 1);

		material->SetTexture("_GlobalNormalMap", uavTex);

		MeshData* meshData = MeshGenerator::GenerateTerrainMesh(hMap, meshSettings, levelOfDetail);

		//delete hMap;

		SpatialPtr testModel = New(Spatial);
		testModel->Position = Vector3(x, 0, y);

		RendererPtr voxelRender = testModel->AddComponent<Renderer>();
		voxelRender->TestColor = MakeRGB(200, 200, 200).toVec3();
		voxelRender->Material = material;
		Mesh* mesh2 = meshData->CreateMesh();
		voxelRender->SetMesh(mesh2);
		rm->MainScene->AddChild(testModel);
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

		Graphics::ReleaseRenderTarget("Sky");
		Graphics::CreateRenderTarget("Sky", NVRHI::Format::RGBA8_UNORM, width, height, NVRHI::Color(0.0f), sampleCount);
	}

	inline void DeviceDestroyed() override
	{
		Log("MainRenderView::DeviceDestroyed");

		_SkyMaterial.reset();

		Graphics::Destroy();
	}

	float _Time = 0;

	void Render(RenderTargetView RTV) override
	{
		ID3D11Resource* pMainResource = NULL;
		RTV->GetResource(&pMainResource);
		NVRHI::TextureHandle mainRenderTarget = _renderInterface->getHandleForTexture(pMainResource);
		pMainResource->Release();

		rsd->Render(rm);

		Graphics::Composite(_SkyMaterial, [this](NVRHI::DrawCallState& state) {
			_SkyMaterial->SetVector2("g_ViewPort", Engine::ScreenSize);
			_SkyMaterial->SetMatrix4("g_InvProjection", glm::inverse(Camera::MainCamera->GetProjectionMatrix()));
			_SkyMaterial->SetMatrix4("g_InvView", glm::inverse(Camera::MainCamera->GetViewMatrix()));
			_SkyMaterial->SetVector3("g_ViewPos", Camera::MainCamera->GameObject->Position);

			Vector3 dir = glm::normalize(Transformable::GetForward(lightObj->GetModelMatrix()));

			_SkyMaterial->SetVector3("g_LightDir", -dir);

			_SkyMaterial->SetFloat("_Time", _Time);

			_SkyMaterial->ApplyParams(state);
		}, "Sky");

		_Time += 0.01f;

		Graphics::Blit("Sky", mainRenderTarget);
		Graphics::Blit(rsd->GetOutputName(), mainRenderTarget);

		MaterialPtr computeMat = Material::CreateOrGet("NormalMapCmp", (File)"Assets/Shaders/Utils/GPU/NormalMapFromHeightMap.hlsl");
		Graphics::Dispatch(computeMat, 16, 16, 1);

		_renderInterface->forgetAboutTexture(pMainResource);
	}

	int ic;

	void Animate(double fElapsedTimeSeconds) override
	{
		_InputManager->Update();
		rm->MainScene->Update();

		Ray r = Camera::MainCamera->GetRay(Engine::ScreenSize.x * 0.5f, Engine::ScreenSize.y * 0.5f);
		CollisionResults results;

		/*if (rm->MainScene->CollideWith(r, results) > 0)
		{
			//box->Position = results.GetClosestCollision().ContactPoint;
			//std::cout << glm::to_string(results.GetClosestCollision().ContactPoint) << std::endl;
		}*/

		MaterialPtr colMat = Material::CreateOrGet("PCollison", (File)"Assets/Shaders/Utils/GPU/ProceduralCollision.hlsl");
		colMat->SetVector3("_Position", Camera::MainCamera->GameObject->Position);
		colMat->SetFloat("_Radius", 0.5);

		colMat->SetVector3("_Origin", r.Origin);
		colMat->SetVector3("_Direction", r.Direction);

		Graphics::Dispatch(colMat, 1, 1, 1);

		NVRHI::BufferHandle buff = colMat->GetBuffer("_Collisions");

		size_t size = sizeof(CollisionBuffer);
		Engine::GetRenderInterface()->readBuffer(buff, &storeColl, &size);

		ic++;

		//lightObj->Rotation.x += 0.1f;
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