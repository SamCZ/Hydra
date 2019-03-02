#include "Hydra/Render/RenderStageDeffered.h"

#include "Hydra/Engine.h"
#include "Hydra/Import/ShaderImporter.h"
#include "Hydra/Core/File.h"
#include "Hydra/Scene/Components/Renderer.h"
#include "Hydra/Scene/Components/Camera.h"
#include "Hydra/Scene/Spatial.h"

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

		_GlobalConstantBuffer = Graphics::CreateConstantBuffer(sizeof(GlobalConstants), "GlobalConstants", PSB_VERTEX, 0);
		_ModelConstantBuffer = Graphics::CreateConstantBuffer(sizeof(ModelConstants), "ModelConstants", PSB_VERTEX, 1);

		_DefaultShader = ShaderImporter::Import("Assets/Shaders/DefaultDeffered.hlsl");
		_CompositeShader = ShaderImporter::Import("Assets/Shaders/DefferedComposite.hlsl");

		
		_BrdfLutTexture = Graphics::CreateRenderTarget("DPBR_BrdfLut", NVRHI::Format::RG16_FLOAT, 512, 512, NVRHI::Color(0.f), 1);
		_BrdfLutSampler = Graphics::CreateSampler("DPBR_BrdfLut", WrapMode::WRAP_MODE_CLAMP, WrapMode::WRAP_MODE_CLAMP, WrapMode::WRAP_MODE_CLAMP);

		ShaderPtr brdfLutShader = ShaderImporter::Import("Assets/Shaders/Utils/BrdfLUT.hlsl");

		Graphics::Composite(brdfLutShader, NULL, "DPBR_BrdfLut");

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

		for (RendererPtr r : activeRenderers)
		{
			if (r->Enabled == false || r->Parent->IsEnabled() == false) continue;

			r->WriteDataToState(Engine::GetRenderInterface().get(), state);
			
			ModelConstants modelData = {};
			modelData.g_ModelMatrix = r->Parent->GetModelMatrix();
			Graphics::WriteConstantBufferDataAndBind(state, "ModelConstants", &modelData);


			Engine::GetRenderInterface()->drawIndexed(state, &r->GetDrawArguments(), 1);

			state.renderState.clearColorTarget = false;
			state.renderState.clearDepthTarget = false;
		}

		Engine::GetRenderInterface()->endRenderingPass();

		//Composite data

		Graphics::Composite(_CompositeShader, [](NVRHI::DrawCallState& state)
		{
			Graphics::BindRenderTarget(state, "DPBR_AlbedoMetallic", 0);
			Graphics::BindRenderTarget(state, "DPBR_NormalRoughness", 1);
			Graphics::BindRenderTarget(state, "DPBR_AO_Emission", 2);
			Graphics::BindRenderTarget(state, "DPBR_Depth", 3);
			Graphics::BindRenderTarget(state, "DPBR_WorldPos", 4);


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