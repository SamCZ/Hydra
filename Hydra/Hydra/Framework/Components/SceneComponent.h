#pragma once

#include "Hydra/Framework/Components/ActorComponent.h"
#include "Hydra/Core/Vector.h"
#include "SceneComponent.generated.h"


HCLASS()
class HYDRA_API HSceneComponent : public HActorComponent
{
	HCLASS_GENERATED_BODY()
private:
	List<HSceneComponent*> Childrens;
public:
	Vector3 Location;
	Vector3 Rotation;
	Vector3 Scale;

	uint8 AbsoluteLocation : 1;
	uint8 AbsoluteRotation : 1;
	uint8 AbsoluteScale : 1;

	uint8 IsVisible : 1;

	HSceneComponent* Parent;
public:
	HSceneComponent();
	virtual ~HSceneComponent();

	virtual void BeginDestroy();

	const List<HSceneComponent*>& GetChildrens() const;
	void GetParentComponents(List<HSceneComponent*>& ParentComponent) const;

	uint32 GetNumChildrenComponents() const;
	HSceneComponent* GetChildComponent(uint32 index) const;

	bool AttachToComponent(HSceneComponent* InParent);
	void DetachFromComponent();

	virtual void OnVisibilityChanged();

	void SetVisibility(bool bNewVisibility, bool bPropagateToChildren = false);

	void ToggleVisibility(bool bPropagateToChildren = false)
	{
		SetVisibility(!IsVisible, bPropagateToChildren);
	}

	virtual Matrix4 GetTransformMatrix() const;

	Vector3 GetForwardVector() const;
	Vector3 GetUpVector() const;
	Vector3 GetLeftVector() const;

private:
	static Vector3 GetRotationColumn(const Matrix4& mat, int i);
};
