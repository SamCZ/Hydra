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

#include "Hydra/Render/MeshBufferDataInternal.h"

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

	Context->GetAssetManager()->OnMeshLoaded += EVENT_ARGS(MainRenderView, OnMeshLoaded, HStaticMesh*);
	Context->GetAssetManager()->OnMeshDeleted += EVENT_ARGS(MainRenderView, OnMeshDeleted, HStaticMesh*);

	Engine->SceneInit();
}

void MainRenderView::OnDestroy()
{

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
		if (!actor->IsActive)
		{
			continue;
		}

		for (HSceneComponent* component : actor->Components)
		{
			UpdateComponent(component, Delta);
		}

		actor->Tick(Delta);
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
	sceneView->Width = Context->ScreenSize.x;
	sceneView->Height = Context->ScreenSize.y;

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

	cmp->SceneView = sceneView;

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

void MainRenderView::OnMeshLoaded(HStaticMesh* mesh)
{
	if (!mesh->RenderData)
	{
		return;
	}

	FStaticMeshRenderData* renderData = mesh->RenderData;

	for (FStaticMeshLODResources& lodResouces : renderData->LODResources)
	{
		if (lodResouces.VertexData.size() == 0)
		{
			continue;
		}

		FMeshBufferDataInternal* bufferData = new FMeshBufferDataInternal();

		NVRHI::BufferDesc vertexBufferDesc;
		vertexBufferDesc.isVertexBuffer = true;
		vertexBufferDesc.byteSize = uint32_t(lodResouces.VertexData.size() * sizeof(VertexBufferEntry));
		bufferData->VertexBuffer = Context->GetRenderInterface()->createBuffer(vertexBufferDesc, &lodResouces.VertexData[0]);

		Log("OnMeshLoaded", mesh->Name, "Vertex buffer created.");

		if (lodResouces.Indices.size() > 0)
		{
			NVRHI::BufferDesc indexBufferDesc;
			indexBufferDesc.isIndexBuffer = true;
			indexBufferDesc.byteSize = uint32_t(lodResouces.Indices.size() * sizeof(unsigned int));
			bufferData->IndexBuffer = Context->GetRenderInterface()->createBuffer(indexBufferDesc, &lodResouces.Indices[0]);

			Log("OnMeshLoaded", mesh->Name, "Index buffer created.");
		}

		lodResouces.InternalBufferData = bufferData;
	}
}

void MainRenderView::OnMeshDeleted(HStaticMesh* mesh)
{
	if (!mesh->RenderData)
	{
		return;
	}

	FStaticMeshRenderData* renderData = mesh->RenderData;

	for (FStaticMeshLODResources& lodResouces : renderData->LODResources)
	{
		FMeshBufferDataInternal* bufferData = lodResouces.InternalBufferData;

		if (bufferData->VertexBuffer)
		{
			Context->GetRenderInterface()->destroyBuffer(bufferData->VertexBuffer);
			bufferData->VertexBuffer = nullptr;

			Log("OnMeshDeleted", mesh->Name, "Vertex buffer destroyed.");
		}

		if (bufferData->IndexBuffer)
		{
			Context->GetRenderInterface()->destroyBuffer(bufferData->IndexBuffer);
			bufferData->IndexBuffer = nullptr;

			Log("OnMeshDeleted", mesh->Name, "Index buffer destroyed.");
		}
	}
}

NVRHI::InputLayoutHandle MainRenderView::GetInputLayoutForMaterial(MaterialInterface * materialInterface)
{


	return nullptr;
}

void MainRenderView::UpdateMaterialGlobalVariables(MaterialInterface* materialInterface, HPrimitiveComponent* component, HCameraComponent* camera)
{
	materialInterface->SetMatrix4("_ProjectionMatrix", camera->GetProjectionMatrix());
	materialInterface->SetMatrix4("_ViewMatrix", camera->GetViewMatrix());

	materialInterface->SetMatrix4("_ModelMatrix", component->GetTransformMatrix());
}

void MainRenderView::UpdateComponent(HSceneComponent* component, float Delta)
{
	component->Tick(Delta);
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

			if (!mesh)
			{
				continue;
			}

			FStaticMeshRenderData* renderData = mesh->RenderData;

			if (!renderData)
			{
				continue;
			}

			List<FStaticMeshLODResources>& lodResource = renderData->LODResources;
			size_t lodCount = lodResource.size();

			int lod = 0; //TODO: Managing lod

			if (lodCount == 0 || lod < 0 || lod > lodCount - 1)
			{
				continue;
			}

			FStaticMeshLODResources& lodData = lodResource[lod];

			if (lodData.VertexData.size() == 0)
			{
				continue;
			}

			FMeshBufferDataInternal* bufferData = lodData.InternalBufferData;

			if (!bufferData)
			{
				continue;
			}

			drawState.SetVertexBuffer(bufferData->VertexBuffer);
			drawState.SetIndexBuffer(bufferData->IndexBuffer);

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
					UpdateMaterialGlobalVariables(materialInterface, cmp, camera);

					drawState.SetMaterial(materialInterface);
					drawState.SetInputLayout(GetInputLayoutForMaterial(materialInterface));

					drawState.Draw(Context->GetRenderInterface(), section.FirstIndex, section.NumTriangles, 0, 1);

					drawState.SetClearFlags(false, false, false);
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
