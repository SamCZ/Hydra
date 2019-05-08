#include "MainRenderView.h"
#include "Hydra/HydraEngine.h"

#include "Hydra/Framework/World.h"
#include "Hydra/Framework/Components/MeshComponent.h"

#include "Hydra/Framework/StaticMesh.h"
#include "Hydra/Framework/StaticMeshResources.h"

#include "Hydra/Framework/Components/SceneComponent.h"
#include "Hydra/Framework/Components/StaticMeshComponent.h"
#include "Hydra/Framework/Components/CameraComponent.h"

#include "Hydra/Render/Material.h"
#include "Hydra/Render/Shader.h"

#include "Hydra/Render/Graphics.h"

#include "Hydra/Render/View/HSceneView.h"

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
	const List<HPrimitiveComponent*>& components = Engine->GetWorld()->GetPrimitiveComponents();

	for (HPrimitiveComponent* cmp : components)
	{
		if (HStaticMeshComponent* staticMeshComponent = cmp->SafeCast<HStaticMeshComponent>())
		{
			HStaticMesh* mesh = staticMeshComponent->StaticMesh;

			if (mesh)
			{

			}
		}
	}


	// Test Render
	/*
	Context->GetGraphics()->Composite(_DefaultMaterial, [](NVRHI::DrawCallState& state) {
		
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
}

void MainRenderView::OnCameraAdded(HCameraComponent* cmp)
{
	HSceneView* sceneView = new HSceneView();


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
		HSceneView* sceneView = iter->second;
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
