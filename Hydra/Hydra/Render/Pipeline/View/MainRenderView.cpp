#include "MainRenderView.h"
#include "Hydra/HydraEngine.h"

#include "Hydra/Framework/World.h"
#include "Hydra/Framework/Components/MeshComponent.h"

#include "Hydra/Framework/StaticMesh.h"
#include "Hydra/Framework/StaticMeshResources.h"

#include "Hydra/Framework/Components/SceneComponent.h"
#include "Hydra/Framework/Components/StaticMeshComponent.h"

#include "Hydra/Render/Material.h"
#include "Hydra/Render/Shader.h"

#include "Hydra/Render/Graphics.h"

void MainRenderView::OnCreated()
{
	LogMethod()

	Engine->InitializeAssetManager(Context->GetAssetManager());
	Engine->SceneInit();

	Context->GetAssetManager()->LoadProjectFiles();

	_DefaultMaterial = Context->GetAssetManager()->GetMaterial("Assets/Materials/Default.mat");

}

void MainRenderView::OnDestroy()
{

}

void MainRenderView::OnRender(NVRHI::TextureHandle mainRenderTarget)
{
	List<HPrimitiveComponent*>& components = Engine->GetWorld()->GetPrimitiveComponents();

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

void MainRenderView::UpdateComponent(HSceneComponent* component, float Delta)
{
	// Component has no tick function for now
}
