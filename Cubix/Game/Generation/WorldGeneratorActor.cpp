#include "WorldGeneratorActor.h"

#include "Hydra/EngineContext.h"

#include "Hydra/Framework/Components/StaticMeshComponent.h"

#include "Hydra/Framework/Utils/ProceduralMesh.h"

void WorldGeneratorActor::InitializeComponents()
{

}

void WorldGeneratorActor::BeginPlay()
{
	ProceduralMesh mesh;

	mesh.addTriangle(Vector3(0, 0, 0), Vector2(0, 0), Vector3(0, 0, 0));

	HStaticMeshComponent* meshComponent = AddComponent<HStaticMeshComponent>("Mesh");
	meshComponent->StaticMesh = mesh.create();

	Engine->GetAssetManager()->OnMeshLoaded.Invoke(meshComponent->StaticMesh);
}

void WorldGeneratorActor::BeginDestroy()
{
	for (HSceneComponent* component : Components)
	{
		if (component->IsA<HStaticMeshComponent>())
		{
			HStaticMeshComponent* meshComponent = component->SafeCast<HStaticMeshComponent>();

			Engine->GetAssetManager()->OnMeshDeleted.Invoke(meshComponent->StaticMesh);
		}
	}
}

void WorldGeneratorActor::Tick(float DeltaTime)
{
}
