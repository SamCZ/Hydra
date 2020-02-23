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
	mesh.addTriangle(Vector3(0, 1, 0), Vector2(0, 0), Vector3(0, 0, 0));
	mesh.addTriangle(Vector3(1, 1, 0), Vector2(0, 0), Vector3(0, 0, 0));

	HStaticMeshComponent* meshComponent = AddComponent<HStaticMeshComponent>("Mesh");
	meshComponent->StaticMesh = mesh.create();

	FStaticMaterial staticMaterial = {  };
	staticMaterial.Material = Engine->GetAssetManager()->GetMaterial("Assets/Materials/World.mat");

	meshComponent->StaticMesh->StaticMaterials.push_back(staticMaterial);

	Engine->GetAssetManager()->OnMeshLoaded.Invoke(meshComponent->StaticMesh);

	SetLocation(0, 0, -10);
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
