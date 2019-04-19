#include "SceneComponent.h"

HSceneComponent::HSceneComponent() : HActorComponent(), Location(0, 0, 0), Rotation(0, 0, 0), Scale(1, 1, 1), AbsoluteLocation(false), AbsoluteRotation(false), AbsoluteScale(false), IsVisible(true), Parent(nullptr)
{

}

void HSceneComponent::BeginDestroy()
{
	DetachFromComponent();

	HActorComponent::BeginDestroy();
}

const List<HSceneComponent*>& HSceneComponent::GetChildrens() const
{
	return Childrens;
}

void HSceneComponent::GetParentComponents(List<HSceneComponent*>& ParentComponent) const
{
	for (HSceneComponent* component : Childrens)
	{
		ParentComponent.push_back(component);
	}
}

uint32 HSceneComponent::GetNumChildrenComponents() const
{
	return uint32(Childrens.size());
}
HSceneComponent* HSceneComponent::GetChildComponent(uint32 index) const
{
	return Childrens[index];
}

bool HSceneComponent::AttachToComponent(HSceneComponent* InParent)
{
	if (Parent == InParent)
	{
		return false;
	}

	DetachFromComponent();

	Parent = InParent;
	InParent->Childrens.push_back(this);

	return true;
}

void HSceneComponent::DetachFromComponent()
{
	if (Parent)
	{
		List_Remove(Parent->Childrens, this);
		Parent = nullptr;
	}
}

void HSceneComponent::OnVisibilityChanged()
{

}

void HSceneComponent::SetVisibility(bool bNewVisibility, bool bPropagateToChildren)
{
	if (IsVisible != bNewVisibility)
	{
		OnVisibilityChanged();
	}

	if (bPropagateToChildren)
	{
		for (HSceneComponent* cmp : Childrens)
		{
			cmp->SetVisibility(bNewVisibility, bPropagateToChildren);
		}
	}
}