#include "MainRenderView.h"
#include "Hydra/HydraEngine.h"

#include "Hydra/Framework/World.h"
#include "Hydra/Framework/Components/MeshComponent.h"

#include "Hydra/Framework/StaticMesh.h"
#include "Hydra/Framework/StaticMeshResources.h"
#include "Hydra/Framework/Character.h"

#include "Hydra/Framework/Components/SceneComponent.h"
#include "Hydra/Framework/Components/StaticMeshComponent.h"
#include "Hydra/Framework/Components/CameraComponent.h"

#include "Hydra/Render/Material.h"
#include "Hydra/Render/Shader.h"

#include "Hydra/Render/Graphics.h"
#include "Hydra/Render/DrawState.h"

#include "Hydra/Render/View/SceneView.h"
#include "Hydra/Render/View/ViewPort.h"

MainRenderView::MainRenderView(EngineContext* context, HydraEngine* engine) : IVisualController(context), Engine(engine), _ScreenRenderViewport(nullptr)
{
}

MainRenderView::~MainRenderView()
{
	if (_ScreenRenderViewport != nullptr)
	{
		delete _ScreenRenderViewport;
		_ScreenRenderViewport = nullptr;
	}
}

void MainRenderView::OnCreated()
{
	Engine->InitializeAssetManager(Context->GetAssetManager());

	Context->GetAssetManager()->LoadProjectFiles();

	_DefaultMaterial = Context->GetAssetManager()->GetMaterial("Assets/Materials/Default.mat");

	Engine->GetWorld()->OnCameraComponentAdded += EVENT_ARGS(MainRenderView, OnCameraAdded, HCameraComponent*);
	Engine->GetWorld()->OnCameraComponentRemoved += EVENT_ARGS(MainRenderView, OnCameraRemoved, HCameraComponent*);

	Engine->SceneInit();
}

void MainRenderView::OnDestroy()
{
	Engine->GetWorld()->OnCameraComponentAdded -= EVENT_NAME(MainRenderView, OnCameraAdded);
	Engine->GetWorld()->OnCameraComponentRemoved -= EVENT_NAME(MainRenderView, OnCameraRemoved);
}

void MainRenderView::OnRender(NVRHI::TextureHandle mainRenderTarget)
{
	ITER(_SceneViewForCameras, it)
	{
		RenderSceneViewFromCamera(it->second, it->first);
	}

	if (_ScreenRenderViewport != nullptr)
	{
		BlitFromViewportToTarget(_ScreenRenderViewport, mainRenderTarget);
	}

	// Test Render
	
	/*Context->GetGraphics()->Composite(_DefaultMaterial, [](NVRHI::DrawCallState& state) {
		
	}, mainRenderTarget);*/
}

void MainRenderView::OnTick(float Delta)
{
	FWorld* world = Engine->GetWorld();

	for (AActor* actor : world->GetActors())
	{
		if (actor->IsActive)
		{
			for (HSceneComponent* component : actor->Components)
			{
				UpdateComponent(component, Delta);
			}

			actor->Tick(Delta);
		}
	}
}

void MainRenderView::OnResize(uint32 width, uint32 height, uint32 sampleCount)
{
	Context->ScreenSize.x = width;
	Context->ScreenSize.y = height;

	_ScreenRenderViewport->Resize(width, height);
}

void MainRenderView::OnCameraAdded(HCameraComponent* cmp)
{
	FSceneView* sceneView = new FSceneView();

	//TODO: Setup automatically FSceneView render targets from HCameraComponent
	sceneView->RenderTexture = Context->GetGraphics()->CreateRenderTarget("CharacterRenderTarget", NVRHI::Format::RGBA8_UNORM, Context->ScreenSize.x, Context->ScreenSize.y, NVRHI::Color(0.0f), 1);

	// If camera component in on character actor, create viewport for the player.
	// Viewport is physical rendering part on the viewport.
	if (cmp->Owner != nullptr && cmp->Owner->IsA<ACharacter>())
	{
		ACharacter* character = cmp->Owner->SafeCast<ACharacter>();

		// This only support one charater !
		// TODO: Multiple character support (split screen)
		if (_ScreenRenderViewport == nullptr)
		{
			//_ScreenRenderViewport = Context->GetRenderManager()->AddOrGetViewPort("CharacterViewPort");
			_ScreenRenderViewport = new FViewPort(Context->ScreenSize.x, Context->ScreenSize.y);
			_ScreenRenderViewport->SetSceneView(sceneView);
		}
	}

	if (_SceneViewForCameras.find(cmp) != _SceneViewForCameras.end())
	{
		LogError("MainRenderView::OnCameraAdded", "Camera already exist !");
		return;
	}

	_SceneViewForCameras[cmp] = sceneView;

	Log("MainRenderView::OnCameraAdded", "SceneView created for " + cmp->Owner->GetClass().GetName());
}

void MainRenderView::OnCameraRemoved(HCameraComponent* cmp)
{
	auto iter = _SceneViewForCameras.find(cmp);

	if (iter != _SceneViewForCameras.end())
	{
		FSceneView* sceneView = iter->second;
		_SceneViewForCameras.erase(cmp);

		delete sceneView;

		Log("MainRenderView::OnCameraRemoved", "SceneView deleted for " + cmp->Owner->GetClass().GetName());
	}
	else
	{
		LogError("MainRenderView::OnCameraRemoved", "Cannot delete camera that is not present !");
	}
}

void MainRenderView::UpdateComponent(HSceneComponent* component, float Delta)
{
	// Component has no tick function for now
}

void MainRenderView::RenderSceneViewFromCamera(FSceneView* view, HCameraComponent* camera)
{
	RenderManager* renderManager = Context->GetRenderManager();

	const List<HPrimitiveComponent*>& components = Engine->GetWorld()->GetPrimitiveComponents();

	//TODO: Batching


	FDrawState drawState;

	drawState.SetClearFlags(true, true, false);
	drawState.SetClearColor(ColorRGBA::White);

	drawState.SetViewPort(Context->ScreenSize);//TODO: Get size of viewport from FSceneView or HCameraComponent
	drawState.SetTargetCount(1);
	drawState.SetTarget(0, view->RenderTexture);

	for (HPrimitiveComponent* cmp : components)
	{
		if (HStaticMeshComponent* staticMeshComponent = cmp->SafeCast<HStaticMeshComponent>())
		{
			HStaticMesh* mesh = staticMeshComponent->StaticMesh;

			if (mesh)
			{
				FStaticMeshRenderData* renderData = mesh->RenderData;

				if (renderData)
				{
					List<FStaticMeshLODResources>& lodResource = renderData->LODResources;
					size_t lodCount = lodResource.size();

					int lod = 0; //TODO: Managing lod

					if (lodCount > 0 && lod >= lodCount - 1)
					{
						FStaticMeshLODResources& lodData = lodResource[lod];

						if (lodData.VertexData.size() == 0)
						{
							continue;
						}

						//TODO: Set vertex buffer to FDrawState

						for (FStaticMeshSection& section : lodData.Sections)
						{
							FStaticMaterial& staticMaterial = mesh->StaticMaterials[section.MaterialIndex];

							MaterialInterface* materialInterface = staticMaterial.Material;

							if (materialInterface == nullptr)
							{
								materialInterface = _DefaultMaterial;
							}

							if (materialInterface != nullptr)
							{
								drawState.SetMaterial(materialInterface);

								drawState.Draw(Context->GetRenderInterface(), section.FirstIndex, section.NumTriangles, 0, 1);
							}
						}
					}
				}
			}
		}
	}
}

void MainRenderView::BlitFromViewportToTarget(FViewPort* viewPort, NVRHI::TextureHandle target)
{
	FSceneView* sceneView = viewPort->GetSceneView();

	if (sceneView != nullptr)
	{
		Context->GetGraphics()->Blit(sceneView->RenderTexture, target);
	}
}
