#include "SceneComponent.h"

HSceneComponent::HSceneComponent() : HActorComponent(), Location(0, 0, 0), Rotation(0, 0, 0), Scale(1, 1, 1), AbsoluteLocation(false), AbsoluteRotation(false), AbsoluteScale(false), IsVisible(true), Parent(nullptr)
{

}

HSceneComponent::~HSceneComponent()
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

Matrix4 HSceneComponent::GetTransformMatrix() const
{
	static Vector3 axisX = Vector3(1, 0, 0);
	static Vector3 axisY = Vector3(0, 1, 0);
	static Vector3 axisZ = Vector3(0, 0, 1);

	Matrix4 rotation = Matrix4();

	rotation *= glm::rotate(glm::radians(Rotation.z), axisZ);
	rotation *= glm::rotate(glm::radians(Rotation.y), axisY);
	rotation *= glm::rotate(glm::radians(Rotation.x), axisX);

	Matrix4 transform = (glm::translate(Location) * rotation) * glm::scale(Scale);

	if (Parent != nullptr)
	{
		Matrix4& parentTransform = Parent->GetTransformMatrix();

		return parentTransform * transform;
	}

	return transform;
}

Vector3 HSceneComponent::GetForwardVector() const
{
	return GetRotationColumn(GetTransformMatrix() , 2);
}

Vector3 HSceneComponent::GetUpVector() const
{
	return GetRotationColumn(GetTransformMatrix(), 1);
}

Vector3 HSceneComponent::GetLeftVector() const
{
	return GetRotationColumn(GetTransformMatrix(), 0);
}

Vector3 HSceneComponent::GetRotationColumn(const Matrix4 & mat, int i)
{
	Matrix4 trns = glm::transpose(mat);

	Vector3 store;

	store.x = trns[i][0];
	store.y = trns[i][1];
	store.z = trns[i][2];
	return store;
}
