#include "Hydra/Render/RenderStage/RenderStageDeffered.h"

#include "Hydra/Engine.h"
#include "Hydra/Import/ShaderImporter.h"
#include "Hydra/Core/File.h"
#include "Hydra/Scene/Components/Renderer.h"
#include "Hydra/Scene/Components/Camera.h"
#include "Hydra/Scene/Components/Light.h"
#include "Hydra/Scene/Spatial.h"
#include "Hydra/Render/Mesh.h"
#include "Hydra/Import/TextureImporter.h"
#include "Hydra/Render/Pipeline/BindingHelpers.h"

#include <algorithm>
#include <random>

#include "Hydra/Render/TestVars.h"

namespace Hydra {

	static float AO_Radius = 0.085f;
	static float AO_Bias = 0.025f;
	static float AO_Intensity = 1.0f;
	static bool AO_Preview = true;

	static const UINT ShadowMapSize = 2048;

	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	struct alignas(16) DirLight : public StorageStruct
	{
		Vector4 Color;
		Vector3 Direction;
		float Intensity;
		float DepthBias;
		float ShadowType;
		Vector2 _Pad0;
	};

	struct alignas(16) PointLight
	{
		alignas(16)
		Vector4 DiffuseColor;
		//------------------------
		alignas(16)
		Vector3 Position;
		float Range;
		//-----------------------
		alignas(16)
		Vector3 Attenuate;
		float pad;
		//-----------------------
	};

	static PointLight PointLights[64];

	RenderStageDeffered::RenderStageDeffered()
	{
		MaterialPtr basicInputMaterial = Material::CreateOrGet("Assets/Shaders/Input/DefferedInput.hlsl");

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

		_InputLayout = Graphics::CreateInputLayout("Deffered", SceneLayout, _countof(SceneLayout), basicInputMaterial);

		_DefaultMaterial = Material::CreateOrGet("Assets/Shaders/DefaultDeffered.hlsl");
		_CompositeMaterial = Material::CreateOrGet("Assets/Shaders/DefferedComposite.hlsl");
		
		Graphics::CreateSampler("DefaultSampler");

		//Create BRDF LUT
		_BrdfLutTexture = Graphics::CreateRenderTarget("DPBR_BrdfLut", NVRHI::Format::RG16_FLOAT, 512, 512, NVRHI::Color(0.f), 1);
		_BrdfLutSampler = Graphics::CreateSampler("DPBR_BrdfLut", WrapMode::WRAP_MODE_CLAMP, WrapMode::WRAP_MODE_CLAMP, WrapMode::WRAP_MODE_CLAMP);

		MaterialPtr brdfLutMaterial = Material::CreateOrGet("Assets/Shaders/Utils/PBR/BrdfLUT.hlsl");

		Graphics::Composite(brdfLutMaterial, [](NVRHI::DrawCallState& state) {}, "DPBR_BrdfLut");


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
		//_IrrConv = TextureImporter::Import("Assets/Textures/skybox2IR.dds");

		Renderer* cubeRenderer = new Renderer();
		cubeRenderer->SetMesh(Mesh::CreatePrimitive(PrimitiveType::Box));

		/*if (!envMap->GetDesc().isCubeMap)
		{
			ShaderPtr equirectangularToCubeMapShader = ShaderImporter::Import("Assets/Shaders/Utils/PBR/EquirectangularToCubeMap.hlsl");

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


		MaterialPtr diffuseIBLMaterial = Material::CreateOrGet("Assets/Shaders/Utils/Skybox.hlsl");

		Graphics::CreateRenderTargetCubeMap("DiffuseIBL", NVRHI::Format::RGBA16_FLOAT, 64, 64, NVRHI::Color(0.6f, 0.6f, 0.6f, 1.0f));

		Graphics::RenderCubeMap(diffuseIBLMaterial, "Deffered", Vector2(64, 64), [=](NVRHI::DrawCallState& state, int mipIndex, int faceIdx)
		{
			diffuseIBLMaterial->SetSampler("_DefaultSampler", Graphics::GetSampler("DefaultSampler"));
			diffuseIBLMaterial->SetTexture("_Texture", envMap);

			diffuseIBLMaterial->SetMatrix4("g_ProjectionMatrix", captureProjection);
			diffuseIBLMaterial->SetMatrix4("g_ViewMatrix", captureViews[faceIdx]);

			Graphics::ApplyMaterialParameters(state, diffuseIBLMaterial);

			cubeRenderer->WriteDataToState(state);
			Engine::GetRenderInterface()->drawIndexed(state, &cubeRenderer->GetDrawArguments(), 1);
			
		}, "DiffuseIBL");

		
#pragma endregion

#pragma region Prefilter EnvMap
		unsigned int maxMipLevels = 5;

		MaterialPtr preFilterMaterial = Material::CreateOrGet("Assets/Shaders/Utils/PBR/PreFilter.hlsl");

		Graphics::CreateRenderTargetCubeMap("EnvMap", NVRHI::Format::RGBA16_FLOAT, 256, 256, NVRHI::Color(0.6f, 0.6f, 0.6f, 1.0f), maxMipLevels);

		Graphics::RenderCubeMap(preFilterMaterial, "Deffered", Vector2(), [=](NVRHI::DrawCallState& state, int mipIndex, int faceIdx)
		{
			float mipWidth = 256.0f * powf(0.5f, (float)mipIndex);
			float mipHeight = 256.0f * powf(0.5f, (float)mipIndex);

			state.renderState.viewports[0] = NVRHI::Viewport(mipWidth, mipHeight);

			float roughness = (float)mipIndex / (float)(maxMipLevels - 1);


			preFilterMaterial->SetSampler("_DefaultSampler", Graphics::GetSampler("DefaultSampler"));
			preFilterMaterial->SetTexture("_Texture", envMap);

			preFilterMaterial->SetMatrix4("g_ProjectionMatrix", captureProjection);
			preFilterMaterial->SetMatrix4("g_ViewMatrix", captureViews[faceIdx]);
			
			preFilterMaterial->SetFloat("_Roughness", roughness);

			Graphics::ApplyMaterialParameters(state, preFilterMaterial);

			cubeRenderer->WriteDataToState(state);
			Engine::GetRenderInterface()->drawIndexed(state, &cubeRenderer->GetDrawArguments(), 1);

		}, "EnvMap");
#pragma endregion

#pragma region IrradianceConvolution
		MaterialPtr irrConvMaterial = Material::CreateOrGet("Assets/Shaders/Utils/PBR/IrradianceConvolution.hlsl");

		Graphics::CreateRenderTargetCubeMap("IrradianceConvolution", NVRHI::Format::RGBA16_FLOAT, 128, 128, NVRHI::Color(0.6f, 0.6f, 0.6f, 1.0f));

		Graphics::RenderCubeMap(irrConvMaterial, "Deffered", Vector2(128, 128), [=](NVRHI::DrawCallState& state, int mipIndex, int faceIdx)
		{

			irrConvMaterial->SetSampler("_DefaultSampler", Graphics::GetSampler("DefaultSampler"));
			irrConvMaterial->SetTexture("_Texture", envMap);

			irrConvMaterial->SetMatrix4("g_ProjectionMatrix", captureProjection);
			irrConvMaterial->SetMatrix4("g_ViewMatrix", captureViews[faceIdx]);

			Graphics::ApplyMaterialParameters(state, irrConvMaterial);

			cubeRenderer->WriteDataToState(state);
			Engine::GetRenderInterface()->drawIndexed(state, &cubeRenderer->GetDrawArguments(), 1);

		}, "IrradianceConvolution");
#pragma endregion

		delete cubeRenderer;


		_PostEmissionPreMaterial = Material::CreateOrGet("Assets/Shaders/PostProcess/EmissionPre.hlsl");
		_PostEmissionMaterial = Material::CreateOrGet("Assets/Shaders/PostProcess/Emission.hlsl");

		_PostSSAOMaterial = Material::CreateOrGet("SSAO", "Assets/Shaders/PostProcess/SSAO.hlsl");

		_MultMaterial = Material::CreateOrGet("Assets/Shaders/Mult.hlsl");

		_ShadowMaterial = Material::CreateOrGet("Assets/Shaders/RenderStage/Shadow.hlsl");

		Graphics::CreateShadowCompareSampler("ShadowSampler");
	}

	RenderStageDeffered::~RenderStageDeffered()
	{
	}

	Matrix4 Transform(const Vector3& location, const Vector3& rotation)
	{
		Matrix4 ViewMatrix = glm::mat4();
		// define your up vector
		glm::vec3 upVector = glm::vec3(0, 1, 0);
		// rotate around to a given bearing: yaw
		glm::mat4 camera = glm::rotate(glm::mat4(), glm::radians(rotation.y), upVector);
		// Define the 'look up' axis, should be orthogonal to the up axis
		glm::vec3 pitchVector = glm::vec3(1, 0, 0);
		// rotate around to the required head tilt: pitch
		camera = glm::rotate(camera, glm::radians(rotation.x), pitchVector);

		glm::vec3 rollVector = glm::vec3(0, 0, 1);
		camera = glm::rotate(camera, glm::radians(rotation.z), rollVector);

		// now get the view matrix by taking the camera inverse
		return glm::inverse(camera) * glm::translate(glm::vec3(-location.x, -location.y, -location.z));
	}

	void RenderStageDeffered::RenderLights(List<SharedPtr<Renderer>>& renderers, List<SharedPtr<Light>>& lights)
	{
		LightPtr dirLight = nullptr;
		for (LightPtr light : lights)
		{
			if (light->Type == LightType::Directional)
			{
				dirLight = light;
			}
		}

		static float r = 0;
		
		Matrix4 viewMatrix = Transform(Vector3(0, 15, 0), dirLight->GameObject->Rotation);
		Matrix4 projMatrix = glm::ortho(Test::OR_LEFT, Test::OR_RIGHT, Test::OR_BOTTOM, Test::OR_TOP, Test::OR_NEAR, Test::OR_FAR);
		
		NVRHI::DrawCallState state;
		state.renderState.targetCount = 1;
		state.renderState.targets[0] = Graphics::GetRenderTarget("DirLight_ShadowMa_ColorTest");
		state.renderState.depthTarget = Graphics::GetRenderTarget("DirLight_ShadowMap");
		state.renderState.clearDepthTarget = true;
		state.renderState.clearColorTarget = true;
		state.renderState.viewportCount = 1;
		state.renderState.viewports[0] = NVRHI::Viewport(float(ShadowMapSize), float(ShadowMapSize));
		state.renderState.rasterState.cullMode = NVRHI::RasterState::CULL_NONE;
		state.renderState.rasterState.depthBias = 16;
		state.renderState.rasterState.slopeScaledDepthBias = 4.0f;

		state.inputLayout = _InputLayout;
		state.VS.shader = _ShadowMaterial->GetRawShader(NVRHI::ShaderType::SHADER_VERTEX);
		state.PS.shader = _ShadowMaterial->GetRawShader(NVRHI::ShaderType::SHADER_PIXEL);

		_ShadowMaterial->SetMatrix4("g_ProjectionMatrix", projMatrix);
		_ShadowMaterial->SetMatrix4("g_ViewMatrix", viewMatrix);

		for (int i = 0; i < renderers.size(); i++)
		{
			RendererPtr& r = renderers[i];

			r->WriteDataToState(state);


			if (r->GameObject->IsStatic())
			{
				_ShadowMaterial->SetMatrix4("g_ModelMatrix", r->GameObject->GetStaticModelMatrix());
			}
			else
			{
				_ShadowMaterial->SetMatrix4("g_ModelMatrix", r->GameObject->GetModelMatrix());
			}

			Graphics::ApplyMaterialParameters(state, _ShadowMaterial);



			Engine::GetRenderInterface()->drawIndexed(state, &r->GetDrawArguments(), 1);

			state.renderState.clearColorTarget = false;
			state.renderState.clearDepthTarget = false;
		}


	}

	void RenderStageDeffered::Render(RenderManagerPtr rm)
	{
		List<RendererPtr> activeRenderers = rm->GetRenderersForStage(GetName());
		List<LightPtr> lights = rm->GetLights(GetName());
		 
		LightPtr dirLight = nullptr;
		for (LightPtr light : lights)
		{
			if (light->Type == LightType::Directional)
			{
				dirLight = light;
			}
		}

		
		RenderLights(activeRenderers, lights);

		//if (true) return;

		CameraPtr camera = Camera::MainCamera;

		NVRHI::DrawCallState state;
		Graphics::SetClearFlags(state, MakeRGBAf(0.0f, 0.0f, 0.0f, 0.0f));

		state.renderState.viewportCount = 1;
		state.renderState.viewports[0] = NVRHI::Viewport(float(camera->GetWidth()), float(camera->GetHeight()));

		state.renderState.targetCount = 4;
		state.renderState.targets[0] = Graphics::GetRenderTarget("DPBR_AlbedoMetallic");
		state.renderState.targets[1] = Graphics::GetRenderTarget("DPBR_NormalRoughness");
		state.renderState.targets[2] = Graphics::GetRenderTarget("DPBR_AO_Emission");
		state.renderState.targets[3] = Graphics::GetRenderTarget("DPBR_WorldPos");
		state.renderState.depthTarget = Graphics::GetRenderTarget("DPBR_Depth");

		state.inputLayout = _InputLayout;
		Graphics::SetMaterialShaders(state, _DefaultMaterial);

		state.renderState.depthStencilState.depthEnable = true;
		state.renderState.rasterState.cullMode = NVRHI::RasterState::CULL_NONE;
		//state.renderState.rasterState.fillMode = NVRHI::RasterState::FILL_LINE;

		_DefaultMaterial->SetSampler("DefaultSampler", Graphics::GetSampler("DefaultSampler"));

		_DefaultMaterial->SetMatrix4("g_ProjectionMatrix", camera->GetProjectionMatrix());
		_DefaultMaterial->SetMatrix4("g_ViewMatrix", camera->GetViewMatrix());

		for (int i = 0; i < activeRenderers.size(); i++)
		{
			RendererPtr& r = activeRenderers[i];

			if (r->Enabled == false || r->GameObject->IsEnabled() == false) continue;

			MaterialPtr material = _DefaultMaterial;

			if (r->Material != nullptr)
			{
				material = r->Material;
			}

			if (material != _DefaultMaterial)
			{
				Graphics::SetMaterialShaders(state, material);

				material->SetMatrix4("g_ProjectionMatrix", camera->GetProjectionMatrix());
				material->SetMatrix4("g_ViewMatrix", camera->GetViewMatrix());
			}
			

			_DefaultMaterial->SetTexture("_AlbedoMap", r->Mat.Albedo);
			if (r->Mat.Normal)
			{
				_DefaultMaterial->SetTexture("_NormalMap", r->Mat.Normal);
			}
			if (r->Mat.Roughness)
			{
				_DefaultMaterial->SetTexture("_RoughnessMap", r->Mat.Roughness);
			}
			if (r->Mat.Metallic)
			{
				_DefaultMaterial->SetTexture("_MetallicMap", r->Mat.Metallic);
			}
			if (r->Mat.Opacity)
			{
				_DefaultMaterial->SetTexture("_AOMap", r->Mat.Opacity);
			}

			r->WriteDataToState(state);
			

			if (r->GameObject->IsStatic())
			{
				material->SetMatrix4("g_ModelMatrix", r->GameObject->GetStaticModelMatrix());
			}
			else
			{
				material->SetMatrix4("g_ModelMatrix", r->GameObject->GetModelMatrix());
			}

			material->SetVector3("_Color", r->TestColor);

			Graphics::ApplyMaterialParameters(state, material);



			Engine::GetRenderInterface()->drawIndexed(state, &r->GetDrawArguments(), 1);

			state.renderState.clearColorTarget = false;
			state.renderState.clearDepthTarget = false;
		}


		//Composite data

		Graphics::Composite(_CompositeMaterial, [this, camera, dirLight, lights](NVRHI::DrawCallState& state)
		{
			_CompositeMaterial->SetSampler("DefaultSampler", Graphics::GetSampler("DefaultSampler"));

			//Shadow begin
			Matrix4 viewMatrix = Transform(Vector3(0, 15, 0), dirLight->GameObject->Rotation);
			Matrix4 projMatrix = glm::ortho(Test::OR_LEFT, Test::OR_RIGHT, Test::OR_BOTTOM, Test::OR_TOP, Test::OR_NEAR, Test::OR_FAR);

			_CompositeMaterial->SetSampler("ShadowSampler", Graphics::GetSampler("ShadowSampler"));
			_CompositeMaterial->SetTexture("DirLight_ShadowMap", Graphics::GetRenderTarget("DirLight_ShadowMap"));
			
			_CompositeMaterial->SetFloat("g_rShadowMapSize", 1.0f / ShadowMapSize);
			_CompositeMaterial->SetMatrix4("g_LightViewProjMatrix", projMatrix * viewMatrix);
			_CompositeMaterial->SetFloat("g_Bias", dirLight->DepthBias);

			DirLight lightStorage = {};
			lightStorage.Color = dirLight->Color.toVec4();
			lightStorage.Direction = glm::normalize(Transformable::GetForward(dirLight->GameObject->GetModelMatrix()));
			lightStorage.Intensity = dirLight->Intensity;
			lightStorage.DepthBias = dirLight->DepthBias;
			lightStorage.ShadowType = (int)dirLight->ShadowType;

			_CompositeMaterial->SetStruct("g_DirLight", lightStorage, sizeof(DirLight));

			int nextPointLightIndex = 0;
			for (LightPtr light : lights)
			{
				if (light->Type == LightType::Point)
				{
					PointLight& pl = PointLights[nextPointLightIndex];

					pl.DiffuseColor = light->Color.toVec4();
					pl.Range = light->Range;
					pl.Position = light->GameObject->Position;

					nextPointLightIndex++;
				}
			}

			_CompositeMaterial->SetStructArray("g_PointLights", PointLights, sizeof(PointLight) * 64);
			_CompositeMaterial->SetFloat("g_PointLightCount", nextPointLightIndex);


			//Shadow end

			_CompositeMaterial->SetTexture("AlbedoMetallic", Graphics::GetRenderTarget("DPBR_AlbedoMetallic"));
			_CompositeMaterial->SetTexture("NormalRoughness", Graphics::GetRenderTarget("DPBR_NormalRoughness"));
			_CompositeMaterial->SetTexture("Additional", Graphics::GetRenderTarget("DPBR_AO_Emission"));
			_CompositeMaterial->SetTexture("Depth", Graphics::GetRenderTarget("DPBR_Depth"));
			_CompositeMaterial->SetTexture("WorldPos", Graphics::GetRenderTarget("DPBR_WorldPos"));

			_CompositeMaterial->SetTexture("skyIR", Graphics::GetRenderTarget("IrradianceConvolution"));
			_CompositeMaterial->SetTexture("skyPrefilter", Graphics::GetRenderTarget("EnvMap"));
			_CompositeMaterial->SetTexture("brdfLUT", Graphics::GetRenderTarget("DPBR_BrdfLut"));

			_CompositeMaterial->SetVector3("ViewPos", camera->GameObject->Position);

			Graphics::ApplyMaterialParameters(state, _CompositeMaterial);

		}, "DPBR_Output");

		/*// Post emission
		Graphics::Composite(_PostEmissionPreShader, "DPBR_AO_Emission", "DPBR_POST_Emission_Output");

		Graphics::BlurTexture("DPBR_POST_Emission_Output", "DPBR_POST_EmissionBlurred_Output");
		Graphics::Composite(_PostEmissionShader, [](NVRHI::DrawCallState& state) {
			Graphics::BindRenderTarget(state, "DPBR_Output", 0);
			Graphics::BindRenderTarget(state, "DPBR_POST_EmissionBlurred_Output", 1);
		}, "DPBR_POST_Output");

		bool ssao = false;*/

		// Post ssao
		/*if (ssao)
		{
			Graphics::Composite(_PostSSAOShader, [this, camera](NVRHI::DrawCallState& state)
			{
				Graphics::BindRenderTarget(state, "DPBR_POST_Output", 0);
				Graphics::BindRenderTarget(state, "DPBR_NormalRoughness", 1);
				Graphics::BindRenderTarget(state, "DPBR_WorldPos", 2);

				static Vector4 Samples[64];
				static bool Inited = false;

				ShaderPtr iShader = _PostSSAOShader->GetShader(NVRHI::ShaderType::SHADER_PIXEL);

				if (!Inited)
				{
					Inited = true;

					std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
					std::default_random_engine generator;
					for (unsigned int i = 0; i < 64; ++i)
					{
						glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
						sample = glm::normalize(sample);
						sample *= randomFloats(generator);
						float scale = float(i) / 64.0f;

						// scale samples s.t. they're more aligned to center of kernel
						scale = lerp(0.1f, 1.0f, scale * scale);
						sample *= scale;

						Samples[i] = Vector4(sample, 0.0);
					}

					std::cout << "Generated : " << sizeof(Samples) << std::endl;


				}


				iShader->SetVariable("g_Projection", camera->GetProjectionMatrix());
				iShader->SetVariable("g_Samples", &Samples, sizeof(Samples));
				iShader->SetVariable("g_View", camera->GetViewMatrix());

				iShader->SetVariable("g_Settings", Vector4(AO_Radius, AO_Bias, AO_Preview ? 1.0 : 0.0, AO_Intensity));

				iShader->UploadVariableData();
				iShader->BindConstantBuffers(state.PS);

			}, "DPBR_POST_SSAO");

			Graphics::BlurTexture("DPBR_POST_SSAO", "DPBR_POST_SSAOBlurred");

			Graphics::Composite(_MultShader, [this, camera](NVRHI::DrawCallState& state)
			{
				Graphics::BindRenderTarget(state, "DPBR_Output", 0);
				Graphics::BindRenderTarget(state, "DPBR_POST_SSAOBlurred", 1);
			}, "DPBR_POST_Output");
		}
		else
		{
			Graphics::Blit("DPBR_Output", "DPBR_POST_Output");
		}*/
	}

	void RenderStageDeffered::AllocateViewDependentResources(uint32 width, uint32 height, uint32 sampleCount)
	{
		Graphics::ReleaseRenderTarget("DPBR_AlbedoMetallic");
		Graphics::ReleaseRenderTarget("DPBR_NormalRoughness");
		Graphics::ReleaseRenderTarget("DPBR_AO_Emission");
		Graphics::ReleaseRenderTarget("DPBR_WorldPos");
		Graphics::ReleaseRenderTarget("DPBR_Depth");

		Graphics::ReleaseRenderTarget("DPBR_Output");
		Graphics::ReleaseRenderTarget("DPBR_POST_Output");
		Graphics::ReleaseRenderTarget("DPBR_POST_Emission_Output");
		Graphics::ReleaseRenderTarget("DPBR_POST_EmissionBlurred_Output");
		Graphics::ReleaseRenderTarget("DPBR_POST_SSAO");
		Graphics::ReleaseRenderTarget("DPBR_POST_SSAOBlurred");

		Graphics::ReleaseRenderTarget("DirLight_ShadowMap");
		Graphics::ReleaseRenderTarget("DirLight_ShadowMa_ColorTest");

		// WARNING CLEAR VALUE DOEST WORK ! TARGET IS CLEARED BY DrawCallArguments clear color value !
		
		Graphics::CreateRenderTarget("DirLight_ShadowMap", NVRHI::Format::D24S8, ShadowMapSize, ShadowMapSize, NVRHI::Color(1.f, 0.f, 0.f, 0.f), 1);
		Graphics::CreateRenderTarget("DirLight_ShadowMa_ColorTest", NVRHI::Format::RGBA8_UNORM, ShadowMapSize, ShadowMapSize, NVRHI::Color(0.f, 0.f, 0.f, 1.f), 1);

		TexturePtr albedoMetallic = Graphics::CreateRenderTarget("DPBR_AlbedoMetallic", NVRHI::Format::RGBA8_UNORM, width, height, NVRHI::Color(0.f), sampleCount);
		TexturePtr normalRoughness = Graphics::CreateRenderTarget("DPBR_NormalRoughness", NVRHI::Format::RGBA16_FLOAT, width, height, NVRHI::Color(0.f), sampleCount);
		TexturePtr aoEmission = Graphics::CreateRenderTarget("DPBR_AO_Emission", NVRHI::Format::RGBA16_FLOAT, width, height, NVRHI::Color(0.0f, 0.0, 0.0, 0.0), sampleCount);
		TexturePtr worldPos = Graphics::CreateRenderTarget("DPBR_WorldPos", NVRHI::Format::RGBA16_FLOAT, width, height, NVRHI::Color(0.f), sampleCount);
		TexturePtr depth = Graphics::CreateRenderTarget("DPBR_Depth", NVRHI::Format::D24S8, width, height, NVRHI::Color(1.f, 0.f, 0.f, 0.f), sampleCount);

		Graphics::CreateRenderTarget("DPBR_Output", NVRHI::Format::RGBA16_FLOAT, width, height, NVRHI::Color(0.f), sampleCount);
		Graphics::CreateRenderTarget("DPBR_POST_Output", NVRHI::Format::RGBA8_UNORM, width, height, NVRHI::Color(0.f), sampleCount);

		Graphics::CreateRenderTarget("DPBR_POST_Emission_Output", NVRHI::Format::RGBA8_UNORM, width, height, NVRHI::Color(0.f), sampleCount);
		Graphics::CreateRenderTarget("DPBR_POST_EmissionBlurred_Output", NVRHI::Format::RGBA8_UNORM, width, height, NVRHI::Color(0.f), sampleCount);

		Graphics::CreateRenderTarget("DPBR_POST_SSAO", NVRHI::Format::RGBA8_UNORM, width, height, NVRHI::Color(0.f), sampleCount);
		Graphics::CreateRenderTarget("DPBR_POST_SSAOBlurred", NVRHI::Format::RGBA8_UNORM, width, height, NVRHI::Color(0.f), sampleCount);

		TextureLayout->DeleteGroup("DefferedPBRPipeline");
		TextureLayout->BeginGroup("DefferedPBRPipeline");

		TextureLayout->Add("Albedo", "ALBEDO_LAYOUT", albedoMetallic, "xyz");
		TextureLayout->Add("Normal", "NORMAL_LAYOUT", albedoMetallic, "xyz");
		TextureLayout->Add("Emission", "EMISSION_LAYOUT", albedoMetallic, "yzw");
		TextureLayout->Add("WorldPos", "WORLDPOS_LAYOUT", albedoMetallic, "xyz");
		TextureLayout->Add("Depth", "DEPTH_LAYOUT", albedoMetallic, "r");

		TextureLayout->EndGroup();
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