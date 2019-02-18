#include <iostream>
#include <algorithm>

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

#include "Hydra/Import/ShaderImporter.h"
#include "Hydra/Render/Shader.h"

#include "Hydra/Import/TextureImporter.h"
#include "Hydra/Import/MeshImporter.h"

#include "Hydra/Engine.h"
#include "Hydra/Render/Mesh.h"



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

		ImGui::Text("yo");

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

__declspec(align(16)) struct BasicConstantBuffer
{
	Matrix4 ProjectionMatrix;
	Matrix4 ViewMatrix;
	//Matrix4 ModelMatrix;
};

class MainRenderView : public IVisualController
{
private:
	SharedPtr<NVRHI::RendererInterfaceD3D11> _renderInterface;

	Shader* _mainSahder;
	Shader* _blitShader;
	NVRHI::InputLayoutHandle _mainInputLayout;
	NVRHI::SamplerHandle m_pDefaultSamplerState;
	NVRHI::TextureHandle textur;

	NVRHI::TextureHandle sceneTarget;
	NVRHI::TextureHandle depthTarget;

	Spatial* testModel;
	Spatial* quadModel;

	NVRHI::ConstantBufferHandle _basicConstantBuffer;

	Camera* camera;

public:
	inline HRESULT DeviceCreated()
	{
		_renderInterface = MakeShared<NVRHI::RendererInterfaceD3D11>(&g_ErrorCallback, _deviceManager->GetImmediateContext());

		Engine::SetRenderInterface(_renderInterface);
		Engine::SetDeviceManager(_deviceManager);

		quadModel = new Spatial("yo");

		camera = quadModel->AddComponent<Camera>();
		camera->Start();

		quadModel->Position.z = 10;

		camera->Update();


		Renderer* _renderer = quadModel->AddComponent<Renderer>();
		Mesh* mesh = new Mesh();
		mesh->Vertices = {
			{ -0.25, -0.5, 0 },
			{ -0.25, 0.5, 0 },
			{ 0.25, 0.5, 0 },
			{ 0.25, -0.5, 0 }
		};
		mesh->TexCoords = {
			{0, 0},
			{0, 1},
			{1, 1},
			{1, 0}
		};
		mesh->Indices = {
			0, 1, 2, 0, 2, 3
		};
		_renderer->SetMesh(mesh);

		_mainSahder = ShaderImporter::Import("Assets/Shaders/basic.hlsl");
		_blitShader = ShaderImporter::Import("Assets/Shaders/blit.hlsl");

		textur = TextureImporter::Import("Assets/Textures/Grassblock_02.dds");

		testModel = Meshimporter::Import("IndustryEmpire/Models/03.fbx", MeshImportOptions());
		//testModel->Scale = Vector3(0.01f, 0.01f, 0.01f);
		testModel->PrintHiearchy();

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

		return S_OK;
	}

	inline void BackBufferResized(uint32_t width, uint32_t height, uint32_t sampleCount) override
	{
		depthTarget = CreateViewportTarget("DPBR_Depth", NVRHI::Format::D24S8, width, height, NVRHI::Color(1.f, 0.f, 0.f, 0.f), sampleCount);
		sceneTarget = CreateViewportTarget("DPBR_AlbedoMetallic", NVRHI::Format::RGBA8_UNORM, width, height, NVRHI::Color(0.f), sampleCount);
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

	void RenderSpatial(Spatial* spatial, NVRHI::DrawCallState& state)
	{
		Renderer* renderer = spatial->GetComponent<Renderer>();

		if (renderer)
		{
			//std::cout << "Rendering: " << spatial->Name << " - " << ToString(renderer->GetDrawArguments().vertexCount) << std::endl;

			renderer->WriteDataToState(_renderInterface.get(), state);

			static BasicConstantBuffer basicConstants = {};
			basicConstants.ProjectionMatrix = camera->GetProjectionMatrix();
			basicConstants.ViewMatrix = camera->GetViewMatrix();
			//basicConstants.ModelMatrix = spatial->GetModelMatrix();
			_renderInterface->writeConstantBuffer(_basicConstantBuffer, &basicConstants, sizeof(BasicConstantBuffer));
			NVRHI::BindConstantBuffer(state.VS, 0, _basicConstantBuffer);

			
			_renderInterface->drawIndexed(state, &renderer->GetDrawArguments(), 1);

			state.renderState.clearColorTarget = false;
			state.renderState.clearDepthTarget = false;
		}

		for (Spatial* child : spatial->GetChilds())
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

		camera->Parent->Rotation.y += 0.5f;
		camera->Update();

		{
			_renderInterface->beginRenderingPass();

			NVRHI::DrawCallState state;
			state.renderState.clearColor = NVRHI::Color(0.2f, 0.2f, 0.2f, 1.0f);
			state.renderState.clearColorTarget = true;
			state.renderState.clearDepthTarget = true;

			state.renderState.viewportCount = 1;
			state.renderState.viewports[0] = NVRHI::Viewport(float(1280), float(720));

			state.renderState.targetCount = 1;
			state.renderState.targets[0] = sceneTarget;
			state.renderState.depthTarget = depthTarget;

			state.inputLayout = _mainInputLayout;
			state.VS.shader = _mainSahder->GetShader(NVRHI::ShaderType::SHADER_VERTEX);
			state.PS.shader = _mainSahder->GetShader(NVRHI::ShaderType::SHADER_PIXEL);

			
			
			NVRHI::BindTexture(state.PS, 0, textur);
			NVRHI::BindSampler(state.PS, 0, m_pDefaultSamplerState);

			state.renderState.depthStencilState.depthEnable = false;
			state.renderState.rasterState.cullMode = NVRHI::RasterState::CULL_NONE;

			RenderSpatial(testModel, state);

			_renderInterface->endRenderingPass();

			Blit(sceneTarget, mainRenderTarget);
		}

		_renderInterface->forgetAboutTexture(pMainResource);
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