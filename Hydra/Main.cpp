#include <iostream>
#include <algorithm>
#include <random>

#include "Hydra/Core/SmartPointer.h"

#include "Hydra/Render/Pipeline/DeviceManager11.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI_D3D11.h"
#include "Hydra/Render/Pipeline/BindingHelpers.h"

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

#include "Hydra/Import/ShaderImporter.h"
#include "Hydra/Render/Shader.h"

#include "Hydra/Import/TextureImporter.h"
#include "Hydra/Import/MeshImporter.h"

#include "Hydra/Engine.h"
#include "Hydra/Render/Mesh.h"

#include "Hydra/Core/FastNoise.h"
#include "Hydra/Core/Polygonise.h"

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

		ImGui::Text(ToString(Engine::GetDeviceManager()->GetAverageFrameTime()).c_str());

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

#define NX 100
#define NY 100
#define NZ 100

class MainRenderView : public IVisualController
{
private:
	SharedPtr<NVRHI::RendererInterfaceD3D11> _renderInterface;

	ShaderPtr _mainSahder;
	ShaderPtr _blitShader;
	ShaderPtr _ssaoShader;

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

	WindowsInputManagerPtr _InputManager;

	CameraPtr camera;

public:
	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	Vector3 ComputeTriangleNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3)
	{
		Vector3 U = p2 - p1;
		Vector3 V = p3 - p1;
		float x = (U.y * V.z) - (U.z * V.y);
		float y = (U.z * V.x) - (U.x * V.z);
		float z = (U.x * V.y) - (U.y * V.x);
		return glm::normalize(Vector3(x, y, z));
	}

	int mix(uint32_t a, uint32_t b, uint32_t c)
	{
		a = a - b;  a = a - c;  a = a ^ (c >> 13);
		b = b - c;  b = b - a;  b = b ^ (a << 8);
		c = c - a;  c = c - b;  c = c ^ (b >> 13);
		a = a - b;  a = a - c;  a = a ^ (c >> 12);
		b = b - c;  b = b - a;  b = b ^ (a << 16);
		c = c - a;  c = c - b;  c = c ^ (b >> 5);
		a = a - b;  a = a - c;  a = a ^ (c >> 3);
		b = b - c;  b = b - a;  b = b ^ (a << 10);
		c = c - a;  c = c - b;  c = c ^ (b >> 15);
		return c;
	}

	inline long hash_ivec2(int x, int y)
	{
		long A = (unsigned long)(x >= 0 ? 2 * (long)x : -2 * (long)x - 1);
		long B = (unsigned long)(y >= 0 ? 2 * (long)y : -2 * (long)y - 1);
		long C = (long)((A >= B ? A * A + A + B : A + B * B) / 2);
		return x < 0 && y < 0 || x >= 0 && y >= 0 ? C : -C - 1;
	}

	struct Edge
	{
		int EdgeIndex;
		List<Vector3[3]> Triangles;
		List<int> TriangleIndices;
	};

	void GenerateRustTexture(SpatialPtr spatial)
	{
		RendererWeakPtr rendererWeak = spatial->GetComponent<Renderer>();

		if (auto renderer = rendererWeak.lock())
		{
			Mesh* mesh = renderer->GetMesh();

			bool hasSmoothNormals = true;
			bool smoothNormalsIndetified = false;

			List<long> alreadyComputedLines;

			for (int i = 0; i < mesh->Indices.size() / 3; i++)
			{
				int index0 = mesh->Indices[i * 3 + 0];
				int index1 = mesh->Indices[i * 3 + 1];
				int index2 = mesh->Indices[i * 3 + 2];

				Vector3& v1 = mesh->Vertices[index0];
				Vector3& v2 = mesh->Vertices[index1];
				Vector3& v3 = mesh->Vertices[index2];

				Vector3 normal = mesh->Normals[index0];

				if (!hasSmoothNormals)
				{
					hasSmoothNormals = true;
					hasSmoothNormals = normal == mesh->Normals[index1] && normal == mesh->Normals[index2];
				}

				if (hasSmoothNormals)
				{
					normal = ComputeTriangleNormal(v1, v2, v3);
				}


			}
		}

		for (SpatialPtr child : spatial->GetChilds())
		{
			GenerateRustTexture(child);
		}
	}

	uint32 KeyCodes[512];
	String KeyNames[512];
	uint32 MaxKeys = 0;

	inline LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		_InputManager->MsgProc(hWnd, uMsg, wParam, lParam);


		return S_OK;
	}



	inline Mesh* CreateVoxelTerrain()
	{
		FastNoise noise;
		noise.SetNoiseType(FastNoise::PerlinFractal);
		float*** data = new float**[NX];
		{
			for (int i = 0; i < NX; i++)
			{
				data[i] = new float*[NY];
				for (int j = 0; j < NY; j++)
				{
					data[i][j] = new float[NZ];
				}
			}
		}

		float v = 0;
		{
			for (int i = 0; i < NX; i++)
			{
				for (int j = 0; j < NY; j++)
				{
					for (int k = 0; k < NZ; k++)
					{
						float val = 1;

						if (i == 0 || i <= NX - 1) val = 0;
						if (j == 0 || j <= NY - 1) val = 0;
						if (k == 0 || k <= NZ - 1) val = 0;

						data[i][j][k] = val;
						//data[i][j][k] = (noise.GetNoise(i * 5.0f, j * 5.0f, k * 5.f) + 0.5f) * 100.0f;
						//std::cout << data[i][j][k] << std::endl;
					}
				}
			}
		}

		int i = 0;
		int j = 0;
		int k = 0;
		int trc = 0;
		List<TRIANGLE> tris;
		TRIANGLE* triangles = new TRIANGLE[10];
		GRIDCELL grid;
		for (i = 0; i < NX - 1; i++)
		{
			for (j = 0; j < NY - 1; j++)
			{
				for (k = 0; k < NZ - 1; k++)
				{
					grid.p[0].x = (float)i;
					grid.p[0].y = (float)j;
					grid.p[0].z = (float)k;
					grid.val[0] = data[i][j][k];
					grid.p[1].x = (float)i + 1;
					grid.p[1].y = (float)j;
					grid.p[1].z = (float)k;
					grid.val[1] = data[i + 1][j][k];
					grid.p[2].x = (float)i + 1;
					grid.p[2].y = (float)j + 1;
					grid.p[2].z = (float)k;
					grid.val[2] = data[i + 1][j + 1][k];
					grid.p[3].x = (float)i;
					grid.p[3].y = (float)j + 1;
					grid.p[3].z = (float)k;
					grid.val[3] = data[i][j + 1][k];
					grid.p[4].x = (float)i;
					grid.p[4].y = (float)j;
					grid.p[4].z = (float)k + 1;
					grid.val[4] = data[i][j][k + 1];
					grid.p[5].x = (float)i + 1;
					grid.p[5].y = (float)j;
					grid.p[5].z = (float)k + 1;
					grid.val[5] = data[i + 1][j][k + 1];
					grid.p[6].x = (float)i + 1;
					grid.p[6].y = (float)j + 1;
					grid.p[6].z = (float)k + 1;
					grid.val[6] = data[i + 1][j + 1][k + 1];
					grid.p[7].x = (float)i;
					grid.p[7].y = (float)j + 1;
					grid.p[7].z = (float)k + 1;
					grid.val[7] = data[i][j + 1][k + 1];


					int triCount = Polygonise(grid, 1, triangles);
					trc += triCount;
					for (int n = 0; n < triCount; n++)
					{
						tris.push_back(triangles[n]);
					}
				}
			}
		}
		delete[] triangles;
		unsigned int vertNextId = 0;

		Mesh* mesh = new Mesh();

		for (int i = 0; i < tris.size(); i++)
		{
			TRIANGLE& tri = tris[i];

			Vector3 normal = ComputeTriangleNormal(tri.p[0], tri.p[1], tri.p[2]);

			for (int j = 0; j < 3; j++)
			{
				mesh->Vertices.push_back(tri.p[j]);
				mesh->Normals.push_back(normal);
				mesh->Indices.push_back(vertNextId++);
			}
		}

		return mesh;
	}

	inline void Escape()
	{
		_InputManager->ToggleMouseCapture();
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

		textur = TextureImporter::Import("Assets/Textures/Grassblock_02.dds");
		testModel = Meshimporter::Import("IndustryEmpire/Models/BrickFactory.fbx", MeshImportOptions());
		testModel->Scale = Vector3(0.01f, 0.01f, 0.01f);

		/*testModel = New(Spatial);
		RendererPtr voxelRender = testModel->AddComponent<Renderer>();
		Mesh* mesh2 = CreateVoxelTerrain();
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

		//ShowCursor(FALSE);
		
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

		

		return S_OK;
	}

	inline void BackBufferResized(uint32_t width, uint32_t height, uint32_t sampleCount) override
	{
		depthTarget = CreateViewportTarget("DPBR_Depth", NVRHI::Format::D24S8, width, height, NVRHI::Color(1.f, 0.f, 0.f, 0.f), sampleCount);
		sceneTarget = CreateViewportTarget("DPBR_AlbedoMetallic", NVRHI::Format::RGBA8_UNORM, width, height, NVRHI::Color(0.f), sampleCount);
		normalTarget = CreateViewportTarget("DPBR_Normal", NVRHI::Format::RGBA16_FLOAT, width, height, NVRHI::Color(0.f), sampleCount);
		positionTarget = CreateViewportTarget("DPBR_Pos", NVRHI::Format::RGBA16_FLOAT, width, height, NVRHI::Color(0.f), sampleCount);
	}

	NVRHI::TextureHandle CreateViewportTarget(std::string name, const NVRHI::Format::Enum& format, UINT width, UINT height, const NVRHI::Color& clearColor, UINT sampleCount)
	{

		NVRHI::TextureDesc gbufferDesc;
		gbufferDesc.width = width;
		gbufferDesc.height = height;
		gbufferDesc.isRenderTarget = true;
		gbufferDesc.useClearValue = true;
		gbufferDesc.sampleCount = sampleCount;
		gbufferDesc.disableGPUsSync = true;

		gbufferDesc.format = format;
		gbufferDesc.clearValue = clearColor;
		gbufferDesc.debugName = name.c_str();
		NVRHI::TextureHandle handle = _renderInterface->createTexture(gbufferDesc, NULL);
		return handle;
	}


	inline void DeviceDestroyed() override
	{
		_renderInterface->destroyConstantBuffer(_basicConstantBuffer);
		_renderInterface->destroyInputLayout(_mainInputLayout);
	}

	void RenderSpatial(SpatialPtr spatial, NVRHI::DrawCallState& state)
	{
		RendererPtr renderer = spatial->GetComponent<Renderer>();

		if (renderer)
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
			state.renderState.clearColor = NVRHI::Color(0.2f, 0.2f, 0.2f, 1.0f);
			state.renderState.clearColorTarget = true;
			state.renderState.clearDepthTarget = true;

			state.renderState.viewportCount = 1;
			state.renderState.viewports[0] = NVRHI::Viewport(float(1280), float(720));

			state.renderState.targetCount = 3;
			state.renderState.targets[0] = sceneTarget;
			state.renderState.targets[1] = normalTarget;
			state.renderState.targets[2] = positionTarget;
			state.renderState.depthTarget = depthTarget;

			state.inputLayout = _mainInputLayout;
			state.VS.shader = _mainSahder->GetShader(NVRHI::ShaderType::SHADER_VERTEX);
			state.PS.shader = _mainSahder->GetShader(NVRHI::ShaderType::SHADER_PIXEL);


			NVRHI::BindTexture(state.PS, 0, textur);
			NVRHI::BindSampler(state.PS, 0, m_pDefaultSamplerState);

			state.renderState.depthStencilState.depthEnable = true;
			state.renderState.rasterState.cullMode = NVRHI::RasterState::CULL_NONE;

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


	std::string title = "Hydra";

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