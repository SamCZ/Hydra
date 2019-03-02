#if 0

#include <iostream>
#include <algorithm>
#include <random>

#include "Hydra/Core/SmartPointer.h"

#include "Hydra/Render/Pipeline/DeviceManager11.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI_D3D11.h"
#include "Hydra/Render/Pipeline/BindingHelpers.h"
#include "Hydra/Render/Graphics.h"

#include "ImGui/imgui.h"
#include "ImGui/ImGuizmo.h"
//#include "ImGui/imgui_helper.h"

#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_style.h"

#include "Hydra/Core/File.h"

#include "Hydra/Scene/Spatial.h"
#include "Hydra/Scene/Components/Camera.h"
#include "Hydra/Scene/Components/Renderer.h"
#include "Hydra/Scene/Components/Movement/FirstPersonController.h"
#include "Hydra/Scene/Components/LodGroup.h"

#include "Hydra/Import/ShaderImporter.h"
#include "Hydra/Render/Shader.h"

#include "Hydra/Import/TextureImporter.h"
#include "Hydra/Import/MeshImporter.h"

#include "Hydra/Engine.h"
#include "Hydra/Render/Mesh.h"

#include "Hydra/Core/FastNoise.h"
#include "Hydra/Core/Polygonise.h"
#include "Hydra/Core/Random.h"
#include "Hydra/Terrain/Erosion.h"

#include "Hydra/Input/Windows/WindowsInputManager.h"

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

using namespace Hydra;

static SharedPtr<DeviceManager> _deviceManager;

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

static float AO_Radius = 0.085f;
static float AO_Bias = 0.025f;
static float AO_Intensity = 1.0f;
static bool AO_Preview = false;
static float tessellationAmount = 1.0;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
class GuiVisualController : public IVisualController
{
private:
	SharedPtr<DeviceManager> _deviceManager;
public:
	GuiVisualController(SharedPtr<DeviceManager> deviceManager) : _deviceManager(deviceManager)
	{

	}

	virtual LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		return 1;
	}

	virtual void Animate(double fElapsedTimeSeconds) override {}

	virtual void Render(RenderTargetView RTV) override
	{
		(void)RTV;
		ImGui_ImplDX11_NewFrame();
		//BeginIds();

		ImGui::Text("AO Settings");

		ImGui::DragFloat("Radius", &AO_Radius, 0.001f);
		ImGui::DragFloat("Bias", &AO_Bias, 0.001f);
		ImGui::DragFloat("Intensity", &AO_Intensity, 0.001f);

		ImGui::Checkbox("Preview", &AO_Preview);

		ImGui::Separator();
		ImGui::Text("Tesselation");
		ImGui::DragFloat("TessellationAmount", &tessellationAmount, 0.01f);

		ImGui::Separator();

		ImGui::Text("Timing");
		ImGui::Text(("Frame time: " + ToString(Engine::GetDeviceManager()->GetAverageFrameTime())).c_str());
		ImGui::Text(("FPS: " + ToString(Engine::GetDeviceManager()->GetFPS())).c_str());

		//ImGui::Image()

		//RestartDragger();
		//_interface->GetInfo().IsGuiFocused = ImGui::IsMouseHoveringAnyWindow() || ImGuizmo::IsUsing();
		ImGui::Render();
	}

	virtual HRESULT DeviceCreated() override
	{
		ImGui_ImplDX11_Init(_deviceManager->GetHWND(), _deviceManager->GetDevice(), _deviceManager->GetImmediateContext());

		ImGui::StyleColorsClassic();
		ImGui_CustomStyle();
		return S_OK;
	}

	virtual void DeviceDestroyed() override
	{
		ImGui_ImplDX11_Shutdown();
	}
};

struct alignas(16) BasicConstantBuffer
{
	alignas(16) Matrix4 ProjectionMatrix;
	alignas(16) Matrix4 ViewMatrix;
	alignas(16) Matrix4 ModelMatrix;
	alignas(16) Matrix3 NormalMatrix;
	alignas(16) Vector2 _Spacing0;
	alignas(16) Vector3 TestColor;
};

struct alignas(16) SSAO_CB
{
	alignas(16) Matrix4 Projection;
	alignas(16) Vector4 Samples[64];
};

struct alignas(16) SSAO_CB_RT
{
	alignas(16) Vector4 RadiusBias;
};

struct alignas(16) TessellationBuffer
{
	float tessellationAmount;
	Vector3 padding;
};

class MainRenderView : public IVisualController
{
private:
	SharedPtr<NVRHI::RendererInterfaceD3D11> _renderInterface;

	ShaderPtr _mainSahder;
	ShaderPtr _blitShader;
	ShaderPtr _ssaoShader;

	ShaderPtr _tesselationShader;

	NVRHI::InputLayoutHandle _mainInputLayout;
	NVRHI::SamplerHandle m_pDefaultSamplerState;
	NVRHI::TextureHandle textur;

	NVRHI::TextureHandle sceneTarget;
	NVRHI::TextureHandle normalTarget;
	NVRHI::TextureHandle depthTarget;
	NVRHI::TextureHandle positionTarget;

	SpatialPtr testModel;
	SpatialPtr quadModel;

	NVRHI::ConstantBufferHandle _basicConstantBuffer;

	NVRHI::ConstantBufferHandle _ssaoCB;
	NVRHI::ConstantBufferHandle _ssaoCB_RB;

	NVRHI::ConstantBufferHandle _tessCB;

	WindowsInputManagerPtr _InputManager;

	CameraPtr camera;

public:
	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

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

	inline HRESULT DeviceCreated()
	{
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

		_InputManager->SetMouseCapture(true);

		_renderInterface = MakeShared<NVRHI::RendererInterfaceD3D11>(&g_ErrorCallback, _deviceManager->GetImmediateContext());

		Engine::SetRenderInterface(_renderInterface);
		Engine::SetDeviceManager(_deviceManager);

		quadModel = New(Spatial, "yo");

		camera = quadModel->AddComponent<Camera>();
		camera->Start();

		quadModel->AddComponent<FirstPersonController>()->Start();

		quadModel->Position.y = 2;
		quadModel->Position.z = 5;

		//quadModel->Rotation.x = -90;

		camera->Update();

		_mainSahder = ShaderImporter::Import("Assets/Shaders/basic.hlsl");
		_blitShader = ShaderImporter::Import("Assets/Shaders/blit.hlsl");
		_ssaoShader = ShaderImporter::Import("Assets/Shaders/ssao.hlsl");

		_tesselationShader = ShaderImporter::Import("Assets/Shaders/tesseleation.hlsl");

		textur = TextureImporter::Import("Assets/Textures/Grassblock_02.dds");
		testModel = Meshimporter::Import("Assets/IndustryEmpire/Models/Pine_002_L.FBX", MeshImportOptions());
		testModel->PrintHiearchy();
		testModel->Scale = Vector3(0.01f, 0.01f, 0.01f);

		testModel->AddComponent<LodGroup>()->Start();
		
		//testModel->FindComponents<Renderer>()[1]->Enabled = false;

		/*for (RendererPtr& r : testModel->FindComponents<Renderer>())
		{
			for (int i = 0; i < 20; i++)
			{
				for (int j = 0; j < 20; j++)
				{
					//r->AddInstance(5 * i, 0, 5 * j);
				}
			}
		}*/

		/*testModel = New(Spatial);
		RendererPtr voxelRender = testModel->AddComponent<Renderer>();
		voxelRender->TestColor = MakeRGB(200, 200, 200).toVec3();
		Mesh* mesh2 = GenerateTerrain();
		voxelRender->SetMesh(mesh2);*/

		//GenerateRustTexture(testModel);

		const NVRHI::VertexAttributeDesc SceneLayout[] = {
			{ "POSITION", 0, NVRHI::Format::RGB32_FLOAT, 0, offsetof(VertexBufferEntry, position), false },
			{ "TEXCOORD", 0, NVRHI::Format::RG32_FLOAT,  0, offsetof(VertexBufferEntry, texCoord), false },
			{ "NORMAL",   0, NVRHI::Format::RGB32_FLOAT, 0, offsetof(VertexBufferEntry, normal),   false },
			{ "TANGENT",  0, NVRHI::Format::RGB32_FLOAT, 0, offsetof(VertexBufferEntry, tangent),  false },
			{ "BINORMAL", 0, NVRHI::Format::RGB32_FLOAT, 0, offsetof(VertexBufferEntry, binormal), false },

			{ "WORLD",    0,    NVRHI::Format::RGBA32_FLOAT, 1, 0, true },
			{ "WORLD",    1,    NVRHI::Format::RGBA32_FLOAT, 1, 16, true },
			{ "WORLD",    2,    NVRHI::Format::RGBA32_FLOAT, 1, 32, true },
			{ "WORLD",    3,    NVRHI::Format::RGBA32_FLOAT, 1, 48, true }
		};
		
		ID3DBlob* vsShaderBlob = _mainSahder->GetShaderBlob(NVRHI::ShaderType::SHADER_VERTEX);
		_mainInputLayout = _renderInterface->createInputLayout(SceneLayout, _countof(SceneLayout), vsShaderBlob->GetBufferPointer(), vsShaderBlob->GetBufferSize());

		if (_mainInputLayout == nullptr)
		{
			std::cout << "Input layout error" << std::endl;
		}


		NVRHI::SamplerDesc samplerDesc;
		samplerDesc.wrapMode[0] = NVRHI::SamplerDesc::WRAP_MODE_WRAP;
		samplerDesc.wrapMode[1] = NVRHI::SamplerDesc::WRAP_MODE_WRAP;
		samplerDesc.wrapMode[2] = NVRHI::SamplerDesc::WRAP_MODE_WRAP;
		samplerDesc.minFilter = samplerDesc.magFilter = samplerDesc.mipFilter = true;
		samplerDesc.anisotropy = 16;
		m_pDefaultSamplerState = _renderInterface->createSampler(samplerDesc);

		_basicConstantBuffer = _renderInterface->createConstantBuffer(NVRHI::ConstantBufferDesc(sizeof(BasicConstantBuffer), "GlobalConstants"), nullptr);

		

		SSAO_CB cb = {};
		cb.Projection = camera->GetProjectionMatrix();

		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
		std::default_random_engine generator;
		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			float scale = float(i) / 64.0;

			// scale samples s.t. they're more aligned to center of kernel
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;

			cb.Samples[i] = Vector4(sample, 0.0);
		}

		_ssaoCB = _renderInterface->createConstantBuffer(NVRHI::ConstantBufferDesc(sizeof(SSAO_CB), nullptr), nullptr);
		_renderInterface->writeConstantBuffer(_ssaoCB, &cb, sizeof(SSAO_CB));

		SSAO_CB_RT cb_rt = {};
		cb_rt.RadiusBias = Vector4(AO_Radius, AO_Bias, 0, AO_Intensity);

		_ssaoCB_RB = _renderInterface->createConstantBuffer(NVRHI::ConstantBufferDesc(sizeof(SSAO_CB_RT), nullptr), nullptr);
		_renderInterface->writeConstantBuffer(_ssaoCB_RB, &cb_rt, sizeof(SSAO_CB_RT));

		_tessCB = _renderInterface->createConstantBuffer(NVRHI::ConstantBufferDesc(sizeof(TessellationBuffer), nullptr), nullptr);

		return S_OK;
	}

	inline void BackBufferResized(uint32_t width, uint32_t height, uint32_t sampleCount) override
	{
		depthTarget = Graphics::CreateRenderTarget("DPBR_Depth", NVRHI::Format::D24S8, width, height, NVRHI::Color(1.f, 0.f, 0.f, 0.f), sampleCount);
		sceneTarget = Graphics::CreateRenderTarget("DPBR_AlbedoMetallic", NVRHI::Format::RGBA8_UNORM, width, height, NVRHI::Color(0.f), sampleCount);
		normalTarget = Graphics::CreateRenderTarget("DPBR_Normal", NVRHI::Format::RGBA16_FLOAT, width, height, NVRHI::Color(0.f), sampleCount);
		positionTarget = Graphics::CreateRenderTarget("DPBR_Pos", NVRHI::Format::RGBA16_FLOAT, width, height, NVRHI::Color(0.f), sampleCount);
	}

	inline void DeviceDestroyed() override
	{
		Graphics::Destroy();

		_renderInterface->destroyConstantBuffer(_basicConstantBuffer);
		_renderInterface->destroyInputLayout(_mainInputLayout);
	}

	void RenderSpatial(SpatialPtr spatial, NVRHI::DrawCallState& state)
	{
		if (!spatial->Enabled) return;
		
		RendererPtr renderer = spatial->GetComponent<Renderer>();

		if (renderer && renderer->Enabled)
		{
			//std::cout << "Rendering: " << spatial->Name << " - " << ToString(renderer->GetDrawArguments().vertexCount) << std::endl;

			renderer->WriteDataToState(_renderInterface.get(), state);

			static BasicConstantBuffer basicConstants = {};
			basicConstants.ProjectionMatrix = camera->GetProjectionMatrix();
			basicConstants.ViewMatrix = camera->GetViewMatrix();
			basicConstants.ModelMatrix = spatial->GetModelMatrix();
			basicConstants.NormalMatrix = glm::transpose(glm::inverse(Matrix3(camera->GetViewMatrix() * spatial->GetModelMatrix())));
			basicConstants.TestColor = renderer->TestColor;
			_renderInterface->writeConstantBuffer(_basicConstantBuffer, &basicConstants, sizeof(BasicConstantBuffer));
			NVRHI::BindConstantBuffer(state.VS, 0, _basicConstantBuffer);
			
			
			_renderInterface->drawIndexed(state, &renderer->GetDrawArguments(), 1);

			state.renderState.clearColorTarget = false;
			state.renderState.clearDepthTarget = false;
		}

		for (SpatialPtr child : spatial->GetChilds())
		{
			RenderSpatial(child, state);
		}
	}

	void Render(RenderTargetView RTV)
	{
		ID3D11Resource* pMainResource = NULL;
		RTV->GetResource(&pMainResource);
		NVRHI::TextureHandle mainRenderTarget = _renderInterface->getHandleForTexture(pMainResource);
		pMainResource->Release();

		//testModel->Rotation.y += 0.1f;
		//camera->Parent->Rotation.y += 0.1f;
		camera->Update();

		{
			_renderInterface->beginRenderingPass();

			NVRHI::DrawCallState state;
			Graphics::SetClearFlags(state, MakeRGBf(0.2f, 0.2f, 0.2f));

			state.renderState.viewportCount = 1;
			state.renderState.viewports[0] = NVRHI::Viewport(float(1280), float(720));

			state.renderState.targetCount = 3;
			state.renderState.targets[0] = sceneTarget;
			state.renderState.targets[1] = normalTarget;
			state.renderState.targets[2] = positionTarget;
			state.renderState.depthTarget = depthTarget;

			state.inputLayout = _mainInputLayout;
			Graphics::SetShader(state, _mainSahder);

			TessellationBuffer tb = {};
			tb.tessellationAmount = tessellationAmount;

			_renderInterface->writeConstantBuffer(_tessCB, &tb, sizeof(TessellationBuffer));
			NVRHI::BindConstantBuffer(state.HS, 0, _tessCB);

			NVRHI::BindTexture(state.PS, 0, textur);
			NVRHI::BindSampler(state.PS, 0, m_pDefaultSamplerState);

			state.renderState.depthStencilState.depthEnable = true;
			state.renderState.rasterState.cullMode = NVRHI::RasterState::CULL_NONE;
			//state.renderState.rasterState.fillMode = NVRHI::RasterState::FillMode::FILL_LINE;

			testModel->GetComponent<LodGroup>()->Update();
			RenderSpatial(testModel, state);

			//RenderSpatial(quadModel, state);

			_renderInterface->endRenderingPass();

			BlitSSAO(mainRenderTarget);
			//Blit(positionTarget, mainRenderTarget);
		}

		_renderInterface->forgetAboutTexture(pMainResource);
	}

	void BlitSSAO(NVRHI::TextureHandle pDest)
	{
		NVRHI::DrawCallState state;

		state.primType = NVRHI::PrimitiveType::TRIANGLE_STRIP;
		state.VS.shader = _ssaoShader->GetShader(NVRHI::ShaderType::SHADER_VERTEX);
		state.PS.shader = _ssaoShader->GetShader(NVRHI::ShaderType::SHADER_PIXEL);

		state.renderState.targetCount = 1;
		state.renderState.targets[0] = pDest;
		state.renderState.viewportCount = 1;
		state.renderState.viewports[0] = NVRHI::Viewport(float(1280), float(720));
		state.renderState.depthStencilState.depthEnable = false;
		state.renderState.rasterState.cullMode = NVRHI::RasterState::CULL_NONE;

		NVRHI::BindTexture(state.PS, 0, sceneTarget);
		NVRHI::BindTexture(state.PS, 1, normalTarget);
		NVRHI::BindTexture(state.PS, 2, positionTarget);

		SSAO_CB_RT cb_rt = {};
		cb_rt.RadiusBias = Vector4(AO_Radius, AO_Bias, AO_Preview ? 1.0 : 0.0, AO_Intensity);
		_renderInterface->writeConstantBuffer(_ssaoCB_RB, &cb_rt, sizeof(SSAO_CB_RT));

		NVRHI::BindConstantBuffer(state.PS, 0, _ssaoCB);
		NVRHI::BindConstantBuffer(state.PS, 1, _ssaoCB_RB);


		NVRHI::DrawArguments args;
		args.vertexCount = 4;
		_renderInterface->draw(state, &args, 1);
	}

	void Blit(NVRHI::TextureHandle pSource, NVRHI::TextureHandle pDest)
	{
		NVRHI::DrawCallState state;

		state.primType = NVRHI::PrimitiveType::TRIANGLE_STRIP;
		state.VS.shader = _blitShader->GetShader(NVRHI::ShaderType::SHADER_VERTEX);
		state.PS.shader = _blitShader->GetShader(NVRHI::ShaderType::SHADER_PIXEL);

		state.renderState.targetCount = 1;
		state.renderState.targets[0] = pDest;
		state.renderState.viewportCount = 1;
		state.renderState.viewports[0] = NVRHI::Viewport(float(1280), float(720));
		state.renderState.depthStencilState.depthEnable = false;
		state.renderState.rasterState.cullMode = NVRHI::RasterState::CULL_NONE;

		NVRHI::BindTexture(state.PS, 0, pSource);

		NVRHI::DrawArguments args;
		args.vertexCount = 4;
		_renderInterface->draw(state, &args, 1);
	}

	void Animate(double fElapsedTimeSeconds)
	{
		_InputManager->Update();


		testModel->Update();
		camera->Parent->Update();
		//std::cout << fElapsedTimeSeconds << std::endl;
		//std::cout << _deviceManager->GetAverageFrameTime() << std::endl;
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

	MainRenderView mainRenderView;
	_deviceManager->AddControllerToFront(&mainRenderView);

	GuiVisualController guiView(_deviceManager);
	_deviceManager->AddControllerToFront(&guiView);


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