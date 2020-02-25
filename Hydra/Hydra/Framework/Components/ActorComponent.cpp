#include "ActorComponent.h"

HActorComponent::HActorComponent() : HObject(), RenderTransformDirty(true), IsActive(false), IsEditorOnly(false), EWorld(nullptr), Owner(nullptr)
{

}

HActorComponent::~HActorComponent()
{
}

void HActorComponent::SetActive(bool newActive)
{
	IsActive = newActive;
}

void HActorComponent::ToggleActive()
{
	IsActive = !IsActive;
}

void HActorComponent::Tick(float Delta)
{
}

void HActorComponent::BeginPlay()
{
}

void HActorComponent::BeginDestroy()
{
}

Matrix4 HActorComponent::GetTransformMatrix()
{
	return Matrix4();
}
