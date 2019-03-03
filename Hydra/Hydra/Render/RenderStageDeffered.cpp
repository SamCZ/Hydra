#include "Hydra/Render/RenderStageDeffered.h"

#include "Hydra/Engine.h"
#include "Hydra/Import/ShaderImporter.h"
#include "Hydra/Core/File.h"
#include "Hydra/Scene/Components/Renderer.h"
#include "Hydra/Scene/Components/Camera.h"
#include "Hydra/Scene/Spatial.h"
#include "Hydra/Render/Mesh.h"
#include "Hydra/Import/TextureImporter.h"
#include "Hydra/Render/Pipeline/BindingHelpers.h"

namespace Hydra
{
	RenderStageDeffered::RenderStageDeffered()
	{
		ShaderPtr basicInputShader = ShaderImporter::Import("Assets/Shaders/Input/DefferedInput.hlsl");

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

		_InputLayout = Graphics::CreateInputLayout("Deffered", SceneLayout, _countof(SceneLayout), basicInputShader);

		 Graphics::CreateConstantBuffer(sizeof(GlobalConstants), "GlobalConstants", PSB_VERTEX, 0);
		 Graphics::CreateConstantBuffer(sizeof(ModelConstants), "ModelConstants", PSB_VERTEX, 1);

		 Graphics::CreateConstantBuffer(sizeof(Float3Constant), "Float3Constant", PSB_PIXEL, 0);

		_DefaultShader = ShaderImporter::Import("Assets/Shaders/DefaultDeffered.hlsl");
		_CompositeShader = ShaderImporter::Import("Assets/Shaders/DefferedComposite.hlsl");

		Graphics::CreateSampler("DefaultSampler");

		//Create BRDF LUT
		_BrdfLutTexture = Graphics::CreateRenderTarget("DPBR_BrdfLut", NVRHI::Format::RG16_FLOAT, 512, 512, NVRHI::Color(0.f), 1);
		_BrdfLutSampler = Graphics::CreateSampler("DPBR_BrdfLut", WrapMode::WRAP_MODE_CLAMP, WrapMode::WRAP_MODE_CLAMP, WrapMode::WRAP_MODE_CLAMP);

		ShaderPtr brdfLutShader = ShaderImporter::Import("Assets/Shaders/Utils/BrdfLUT.hlsl");

		Graphics::Composite(brdfLutShader, NULL, "DPBR_BrdfLut");


		// Convert skybox to lower resolution
#pragma region Diffuse IBL

		glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
		glm::mat4 captureViews[] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),

			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)), // IN HLSL THIS TWO ARE REVERSED
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)), //
			
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};


		TexturePtr envMap = TextureImporter::Import("Assets/Textures/skybox1.dds");
		_IrrConv = TextureImporter::Import("Assets/Textures/skybox1IR.dds");

		Renderer* cubeRenderer = new Renderer();
		cubeRenderer->SetMesh(Mesh::CreatePrimitive(PrimitiveType::Box));

		ShaderPtr diffuseIBLShader = ShaderImporter::Import("Assets/Shaders/Utils/Skybox.hlsl");

		Graphics::CreateRenderTargetCubeMap("DiffuseIBL", NVRHI::Format::RGBA16_FLOAT, 64, 64, NVRHI::Color(0.6f, 0.6f, 0.6f, 1.0f));

		Graphics::RenderCubeMap(diffuseIBLShader, "Deffered", Vector2(64, 64), [=](NVRHI::DrawCallState& state, int mipIndex, int faceIdx)
		{
			
			Graphics::BindSampler(state, "DefaultSampler", 0);
			NVRHI::BindTexture(state.PS, 0, envMap);

			GlobalConstants data = {};
			data.g_ProjectionMatrix = captureProjection;
			data.g_ViewMatrix = captureViews[faceIdx];
			Graphics::WriteConstantBufferDataAndBind(state, "GlobalConstants", &data);

			cubeRenderer->WriteDataToState(state);
			Engine::GetRenderInterface()->drawIndexed(state, &cubeRenderer->GetDrawArguments(), 1);
			
		}, "DiffuseIBL");

		
#pragma endregion

#pragma region Prefilter EnvMap
		unsigned int maxMipLevels = 5;

		ShaderPtr preFilterShader = ShaderImporter::Import("Assets/Shaders/Utils/PreFilter.hlsl");

		Graphics::CreateRenderTargetCubeMap("EnvMap", NVRHI::Format::RGBA16_FLOAT, 256, 256, NVRHI::Color(0.6f, 0.6f, 0.6f, 1.0f), maxMipLevels);

		Graphics::CreateConstantBuffer(sizeof(SingleFloatConstant), "SingleFloatConstant", PSB_PIXEL, 1);

		Graphics::RenderCubeMap(preFilterShader, "Deffered", Vector2(), [=](NVRHI::DrawCallState& state, int mipIndex, int faceIdx)
		{
			unsigned mipWidth = 256 * pow(0.5, mipIndex);
			unsigned mipHeight = 256 * pow(0.5, mipIndex);

			state.renderState.viewports[0] = NVRHI::Viewport(mipWidth, mipHeight);

			float roughness = (float)mipIndex / (float)(maxMipLevels - 1);


			Graphics::BindSampler(state, "DefaultSampler", 0);
			NVRHI::BindTexture(state.PS, 0, envMap);

			GlobalConstants data = {};
			data.g_ProjectionMatrix = captureProjection;
			data.g_ViewMatrix = captureViews[faceIdx];
			Graphics::WriteConstantBufferDataAndBind(state, "GlobalConstants", &data);

			SingleFloatConstant data2 = {};
			data2.Float = roughness;
			Graphics::WriteConstantBufferDataAndBind(state, "SingleFloatConstant", &data2);

			cubeRenderer->WriteDataToState(state);
			Engine::GetRenderInterface()->drawIndexed(state, &cubeRenderer->GetDrawArguments(), 1);

		}, "EnvMap");
#pragma endregion

#pragma region IrradianceConvolution
		ShaderPtr irrConvShader = ShaderImporter::Import("Assets/Shaders/Utils/IrradianceConvolution.hlsl");

		Graphics::CreateRenderTargetCubeMap("IrradianceConvolution", NVRHI::Format::RGBA16_FLOAT, 128, 128, NVRHI::Color(0.6f, 0.6f, 0.6f, 1.0f));

		Graphics::RenderCubeMap(irrConvShader, "Deffered", Vector2(128, 128), [=](NVRHI::DrawCallState& state, int mipIndex, int faceIdx)
		{

			Graphics::BindSampler(state, "DefaultSampler", 0);
			NVRHI::BindTexture(state.PS, 0, envMap);

			GlobalConstants data = {};
			data.g_ProjectionMatrix = captureProjection;
			data.g_ViewMatrix = captureViews[faceIdx];
			Graphics::WriteConstantBufferDataAndBind(state, "GlobalConstants", &data);

			cubeRenderer->WriteDataToState(state);
			Engine::GetRenderInterface()->drawIndexed(state, &cubeRenderer->GetDrawArguments(), 1);

		}, "IrradianceConvolution");
#pragma endregion

		delete cubeRenderer;


		_Albedo = TextureImporter::Import("Assets/Textures/Metal/copper-rock1-alb.dds");
		_Normal = TextureImporter::Import("Assets/Textures/Metal/copper-rock1-normal.dds");
		_Roughness = TextureImporter::Import("Assets/Textures/Metal/copper-rock1-height.dds");
		_Metallic = TextureImporter::Import("Assets/Textures/Metal/copper-rock1-height.dds");
		_AO = TextureImporter::Import("Assets/Textures/Metal/copper-rock1-ao.dds");
	}

	RenderStageDeffered::~RenderStageDeffered()
	{
	}

	void RenderStageDeffered::Render(RenderManagerPtr rm)
	{
		List<RendererPtr> activeRenderers = rm->GetRenderersForStage(GetName());


		CameraPtr camera = Camera::MainCamera;

		Engine::GetRenderInterface()->beginRenderingPass();

		NVRHI::DrawCallState state;
		Graphics::SetClearFlags(state, MakeRGBf(0.2f, 0.2f, 0.2f));

		state.renderState.viewportCount = 1;
		state.renderState.viewports[0] = NVRHI::Viewport(float(camera->GetWidth()), float(camera->GetHeight()));

		state.renderState.targetCount = 4;
		state.renderState.targets[0] = Graphics::GetRenderTarget("DPBR_AlbedoMetallic");
		state.renderState.targets[1] = Graphics::GetRenderTarget("DPBR_NormalRoughness");
		state.renderState.targets[2] = Graphics::GetRenderTarget("DPBR_AO_Emission");
		state.renderState.targets[3] = Graphics::GetRenderTarget("DPBR_WorldPos");
		state.renderState.depthTarget = Graphics::GetRenderTarget("DPBR_Depth");

		state.inputLayout = _InputLayout;
		Graphics::SetShader(state, _DefaultShader);

		state.renderState.depthStencilState.depthEnable = true;
		state.renderState.rasterState.cullMode = NVRHI::RasterState::CULL_NONE;

		GlobalConstants cameraData = {};
		cameraData.g_ProjectionMatrix = camera->GetProjectionMatrix();
		cameraData.g_ViewMatrix = camera->GetViewMatrix();
		Graphics::WriteConstantBufferDataAndBind(state, "GlobalConstants", &cameraData);


		Graphics::BindSampler(state, "DefaultSampler", 0);
		NVRHI::BindTexture(state.PS, 0, _Albedo);
		NVRHI::BindTexture(state.PS, 1, _Normal);
		NVRHI::BindTexture(state.PS, 2, _Roughness);
		NVRHI::BindTexture(state.PS, 3, _Metallic);
		NVRHI::BindTexture(state.PS, 4, _AO);


		ModelConstants modelData = {};

		static Map<int, ConstantBufferPtr> CBuffers;

		for (int i = 0; i < activeRenderers.size(); i++)
		{
			if (CBuffers.find(i) == CBuffers.end())
			{
				CBuffers[i] = Graphics::CreateConstantBuffer(sizeof(ModelConstants), "CB" + ToString(i));
			}

			RendererPtr& r = activeRenderers[i];
			modelData.g_ModelMatrix = r->Parent->GetModelMatrix();
			Engine::GetRenderInterface()->writeConstantBuffer(CBuffers[i], &modelData, sizeof(ModelConstants));
		}

		for (int i = 0; i < activeRenderers.size(); i++)
		{
			RendererPtr& r = activeRenderers[i];

			if (r->Enabled == false || r->Parent->IsEnabled() == false) continue;

			r->WriteDataToState(state);
			
			NVRHI::BindConstantBuffer(state.VS, 1, CBuffers[i]);


			Engine::GetRenderInterface()->drawIndexed(state, &r->GetDrawArguments(), 1);

			state.renderState.clearColorTarget = false;
			state.renderState.clearDepthTarget = false;
		}

		Engine::GetRenderInterface()->endRenderingPass();

		//Composite data

		Graphics::Composite(_CompositeShader, [this, camera](NVRHI::DrawCallState& state)
		{
			Graphics::BindSampler(state, "DefaultSampler", 0);

			Graphics::BindRenderTarget(state, "DPBR_AlbedoMetallic", 0);
			Graphics::BindRenderTarget(state, "DPBR_NormalRoughness", 1);
			Graphics::BindRenderTarget(state, "DPBR_AO_Emission", 2);
			Graphics::BindRenderTarget(state, "DPBR_Depth", 3);
			Graphics::BindRenderTarget(state, "DPBR_WorldPos", 4);

			NVRHI::BindTexture(state.PS, 5, _IrrConv);
			Graphics::BindRenderTarget(state, "EnvMap", 6);
			Graphics::BindRenderTarget(state, "DPBR_BrdfLut", 7);

			Float3Constant data = {};
			data.Vector = camera->Parent->Position;
			Graphics::WriteConstantBufferDataAndBind(state, "Float3Constant", &data);

		}, "DPBR_Output");
	}

	void RenderStageDeffered::AllocateViewDependentResources(uint32 width, uint32 height, uint32 sampleCount)
	{
		Graphics::ReleaseRenderTarget("DPBR_AlbedoMetallic");
		Graphics::ReleaseRenderTarget("DPBR_NormalRoughness");
		Graphics::ReleaseRenderTarget("DPBR_AO_Emission");
		Graphics::ReleaseRenderTarget("DPBR_WorldPos");
		Graphics::ReleaseRenderTarget("DPBR_Depth");

		Graphics::ReleaseRenderTarget("DPBR_Output");

		Graphics::CreateRenderTarget("DPBR_AlbedoMetallic", NVRHI::Format::RGBA8_UNORM, width, height, NVRHI::Color(0.f), sampleCount);
		Graphics::CreateRenderTarget("DPBR_NormalRoughness", NVRHI::Format::RGBA16_FLOAT, width, height, NVRHI::Color(0.f), sampleCount);
		Graphics::CreateRenderTarget("DPBR_AO_Emission", NVRHI::Format::RGBA8_UNORM, width, height, NVRHI::Color(0.f), sampleCount);
		Graphics::CreateRenderTarget("DPBR_WorldPos", NVRHI::Format::RGBA16_FLOAT, width, height, NVRHI::Color(0.f), sampleCount);
		Graphics::CreateRenderTarget("DPBR_Depth", NVRHI::Format::D24S8, width, height, NVRHI::Color(1.f, 0.f, 0.f, 0.f), sampleCount);

		Graphics::CreateRenderTarget("DPBR_Output", NVRHI::Format::RGBA8_UNORM, width, height, NVRHI::Color(0.f), sampleCount);
	}

	String RenderStageDeffered::GetOutputName()
	{
		return "DPBR_Output";
	}

	String RenderStageDeffered::GetDepthOutputName()
	{
		return "DPBR_Depth";
	}

	String RenderStageDeffered::GetName()
	{
		return "Deffered";
	}
}