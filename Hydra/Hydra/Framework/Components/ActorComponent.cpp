#include "ActorComponent.h"

namespace Hydra
{
	HActorComponent::HActorComponent() : HObject(), RenderTransformDirty(true), IsActive(false), IsEditorOnly(false), World(nullptr), Owner(nullptr)
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
}