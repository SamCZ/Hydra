#include "MainRenderView.h"
#include "Hydra/HydraEngine.h"

#include "Hydra/Framework/World.h"
#include "Hydra/Framework/Components/MeshComponent.h"

#include "Hydra/Framework/StaticMesh.h"
#include "Hydra/Framework/StaticMeshResources.h"

#include "Hydra/Framework/Components/StaticMeshComponent.h"

namespace Hydra
{
	void MainRenderView::OnCreated()
	{
		Engine->SceneInit();
	}

	void MainRenderView::OnDestroy()
	{

	}

	void MainRenderView::OnRender(NVRHI::TextureHandle mainRenderTarget)
	{
		List<HPrimitiveComponent*>& components = Engine->GetWorld()->GetPrimitiveComponents();

		for (HPrimitiveComponent* cmp : components)
		{
			if (cmp->IsA<HStaticMeshComponent>())
			{
				
			}
		}
	}

	void MainRenderView::OnTick(float Delta)
	{

	}

	void MainRenderView::OnResize(uint32 width, uint32 height, uint32 sampleCount)
	{

	}
}