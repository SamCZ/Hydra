#include "Hydra/Framework/Actor.h"
#include "Hydra/Framework/World.h"
#include "Hydra/Framework/Components/PrimitiveComponent.h"

AActor::AActor() : HObject(), Engine(nullptr), World(nullptr), RootComponent(nullptr), IsIndestructible(false), IsActive(true), IsEditorOnly(false)
{

}
void AActor::InitializeComponents()
{

}

void AActor::DestroyComponent(HSceneComponent*& component)
{
	if (component)
	{
		if (HPrimitiveComponent* pComponent = dynamic_cast<HPrimitiveComponent*>(component))
		{
			World->UnregisterPrimitiveComponent(pComponent);
		}

		List_Remove(Components, component);

		delete component;
		component = nullptr;
	}
}

void AActor::BeginPlay()
{

}

void AActor::BeginDestroy()
{
}

void AActor::Tick(float DeltaTime)
{

}

void AActor::SetActive(bool newActive)
{
	IsActive = newActive;
}

void AActor::ToggleActive()
{
	IsActive = !IsActive;
}

void AActor::Destroy()
{
	if (!IsIndestructible)
	{
		World->DestroyActor(this);
	}
}

Vector3& AActor::GetLocation()
{
	return RootComponent->Location;
}

Vector3& AActor::GetRotation()
{
	return RootComponent->Rotation;
}

Vector3& AActor::GetScale()
{
	return RootComponent->Scale;
}

void AActor::SetLocation(const Vector3& location)
{
	RootComponent->Location = location;
}

void AActor::SetRotation(const Vector3& rotation)
{
	RootComponent->Rotation = rotation;
}

void AActor::SetScale(const Vector3& scale)
{
	RootComponent->Scale = scale;
}

void AActor::SetLocation(float x, float y, float z)
{
	RootComponent->Location.x = x;
	RootComponent->Location.y = y;
	RootComponent->Location.z = z;
}

void AActor::SetRotation(float x, float y, float z)
{
	RootComponent->Rotation.x = x;
	RootComponent->Rotation.y = y;
	RootComponent->Rotation.z = z;
}

void AActor::SetScale(float x, float y, float z)
{
	RootComponent->Scale.x = x;
	RootComponent->Scale.y = y;
	RootComponent->Scale.z = z;
}

void AActor::AddLocation(const Vector3& location)
{
	RootComponent->Location += location;
}

void AActor::AddRotation(const Vector3& rotation)
{
	RootComponent->Rotation += rotation;
}

void AActor::AddScale(const Vector3& scale)
{
	RootComponent->Scale += scale;
}

void AActor::AddLocation(float x, float y, float z)
{
	RootComponent->Location.x += x;
	RootComponent->Location.y += y;
	RootComponent->Location.z += z;
}

void AActor::AddRotation(float x, float y, float z)
{
	RootComponent->Rotation.x += x;
	RootComponent->Rotation.y += y;
	RootComponent->Rotation.z += z;
}

void AActor::AddScale(float x, float y, float z)
{
	RootComponent->Scale.x += x;
	RootComponent->Scale.y += y;
	RootComponent->Scale.z += z;
}

HGameModeBase* AActor::GetGameMode()
{
	return World->GetGameMode();
}

void AActor::InitilizeComponent(HSceneComponent* component)
{
	component->World = World;
	component->Engine = Engine;
	component->Owner = this;

	if (HPrimitiveComponent* pComponent = dynamic_cast<HPrimitiveComponent*>(component))
	{
		World->RegisterPrimitiveComponent(pComponent);
	}
}