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

#include <algorithm>
#include <random>

namespace Hydra {

	static float AO_Radius = 0.085f;
	static float AO_Bias = 0.025f;
	static float AO_Intensity = 1.0f;
	static bool AO_Preview = true;

	struct alignas(16) SSAO_CB
	{
		alignas(16) Matrix4 Projection;
		alignas(16) Vector4 Samples[64];
	};

	struct alignas(16) SSAO_CB_RT
	{
		alignas(16) Vector4 RadiusBias;
	};

	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	RenderStageDeffered::RenderStageDeffered()
	{
		TechniquePtr basicInputShader = _TECH("Assets/Shaders/Input/DefferedInput.hlsl");

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

		 _DefaultShader = Graphics::LoadTechnique("DefaultPBRShader", "Assets/Shaders/DefaultDeffered.hlsl");
		_CompositeShader = _TECH("Assets/Shaders/DefferedComposite.hlsl");
		
		Graphics::CreateSampler("DefaultSampler");

		//Create BRDF LUT
		_BrdfLutTexture = Graphics::CreateRenderTarget("DPBR_BrdfLut", NVRHI::Format::RG16_FLOAT, 512, 512, NVRHI::Color(0.f), 1);
		_BrdfLutSampler = Graphics::CreateSampler("DPBR_BrdfLut", WrapMode::WRAP_MODE_CLAMP, WrapMode::WRAP_MODE_CLAMP, WrapMode::WRAP_MODE_CLAMP);

		TechniquePtr brdfLutShader = _TECH("Assets/Shaders/Utils/BrdfLUT.hlsl");

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


		TexturePtr envMap = TextureImporter::Import("Assets/Textures/skybox3.dds");
		//_IrrConv = TextureImporter::Import("Assets/Textures/skybox2IR.dds");

		Renderer* cubeRenderer = new Renderer();
		cubeRenderer->SetMesh(Mesh::CreatePrimitive(PrimitiveType::Box));

		/*if (!envMap->GetDesc().isCubeMap)
		{
			ShaderPtr equirectangularToCubeMapShader = ShaderImporter::Import("Assets/Shaders/Utils/EquirectangularToCubeMap.hlsl");

			TexturePtr cubeMap = Graphics::CreateRenderTargetCubeMap("Local:Cubemap0", NVRHI::Format::RGBA16_FLOAT, 64, 64, NVRHI::Color(0.6f, 0.6f, 0.6f, 1.0f));

			Graphics::RenderCubeMap(equirectangularToCubeMapShader, "Deffered", Vector2(512, 512), [=](NVRHI::DrawCallState& state, int mipIndex, int faceIdx)
			{

				Graphics::BindSampler(state, "DefaultSampler", 0);
				NVRHI::BindTexture(state.PS, 0, envMap);

				GlobalConstants data = {};
				data.g_ProjectionMatrix = captureProjection;
				data.g_ViewMatrix = captureViews[faceIdx];
				Graphics::WriteConstantBufferDataAndBind(state, "GlobalConstants", &data);

				cubeRenderer->WriteDataToState(state);
				Engine::GetRenderInterface()->drawIndexed(state, &cubeRenderer->GetDrawArguments(), 1);

			}, "Local:Cubemap0");

			envMap = cubeMap
		}*/


		TechniquePtr diffuseIBLShader = _TECH("Assets/Shaders/Utils/Skybox.hlsl");

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

		TechniquePtr preFilterShader = _TECH("Assets/Shaders/Utils/PreFilter.hlsl");

		Graphics::CreateRenderTargetCubeMap("EnvMap", NVRHI::Format::RGBA16_FLOAT, 256, 256, NVRHI::Color(0.6f, 0.6f, 0.6f, 1.0f), maxMipLevels);

		Graphics::CreateConstantBuffer(sizeof(SingleFloatConstant), "SingleFloatConstant", PSB_PIXEL, 1);

		Graphics::RenderCubeMap(preFilterShader, "Deffered", Vector2(), [=](NVRHI::DrawCallState& state, int mipIndex, int faceIdx)
		{
			float mipWidth = 256.0f * powf(0.5f, (float)mipIndex);
			float mipHeight = 256.0f * powf(0.5f, (float)mipIndex);

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
		TechniquePtr irrConvShader = _TECH("Assets/Shaders/Utils/IrradianceConvolution.hlsl");

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


		ShaderImporter::Import("Assets/Shaders/testKeywords.hlsl");
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
		


		ModelConstants modelData = {};

		static Map<int, ConstantBufferPtr> CBuffers;

		for (int i = 0; i < activeRenderers.size(); i++)
		{
			if (CBuffers.find(i) == CBuffers.end())
			{
				CBuffers[i] = Graphics::CreateConstantBuffer(sizeof(ModelConstants), "CB" + ToString(i));
			}

			RendererPtr& r = activeRenderers[i];
			if (r->Parent->IsStatic())
			{
				modelData.g_ModelMatrix = r->Parent->GetStaticModelMatrix();
			}
			else
			{
				modelData.g_ModelMatrix = r->Parent->GetModelMatrix();
			}

			modelData.g_Opacity = r->Mat.Opacity == nullptr ? 0.0f : 1.0f;

			Engine::GetRenderInterface()->writeConstantBuffer(CBuffers[i], &modelData, sizeof(ModelConstants));
		}

		for (int i = 0; i < activeRenderers.size(); i++)
		{
			RendererPtr& r = activeRenderers[i];

			if (r->Enabled == false || r->Parent->IsEnabled() == false) continue;

			NVRHI::BindTexture(state.PS, 0, r->Mat.Albedo);
			if (r->Mat.Normal)
			{
				NVRHI::BindTexture(state.PS, 1, r->Mat.Normal);
			}
			if (r->Mat.Roughness)
			{
				NVRHI::BindTexture(state.PS, 2, r->Mat.Roughness);
			}
			if (r->Mat.Metallic)
			{
				NVRHI::BindTexture(state.PS, 3, r->Mat.Metallic);
			}
			if (r->Mat.Opacity)
			{
				NVRHI::BindTexture(state.PS, 4, r->Mat.Opacity);
			}

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

			//NVRHI::BindTexture(state.PS, 5, _IrrConv);
			Graphics::BindRenderTarget(state, "IrradianceConvolution", 5);
			Graphics::BindRenderTarget(state, "EnvMap", 6);
			Graphics::BindRenderTarget(state, "DPBR_BrdfLut", 7);

			ShaderPtr iShader = _CompositeShader->GetShader(NVRHI::ShaderType::SHADER_PIXEL);

			iShader->SetVariable("ViewPos", camera->Parent->Position);

			iShader->UploadVariableData();
			iShader->BindConstantBuffers(state.PS);


			/*Float3Constant data = {};
			data.Vector = camera->Parent->Position;
			Graphics::WriteConstantBufferDataAndBind(state, "Float3Constant", &data);*/

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