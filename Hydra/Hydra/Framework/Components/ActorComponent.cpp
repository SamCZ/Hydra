#include "ActorComponent.h"

HActorComponent::HActorComponent() : HObject(), RenderTransformDirty(true), IsActive(false), IsEditorOnly(false), World(nullptr), Owner(nullptr)
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

void HActorComponent::BeginPlay()
{
}

void HActorComponent::BeginDestroy()
{
}