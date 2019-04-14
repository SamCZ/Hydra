#pragma once

#include "Hydra/Framework/Components/ActorComponent.h"
#include "Hydra/Core/Vector.h"


namespace Hydra
{
	class HYDRA_API HSceneComponent : public HActorComponent
	{
	public:
		Vector3 Location;
		Vector3 Rotation;
		Vector3 Scale;

		uint8 AbsoluteLocation : 1;
		uint8 AbsoluteRotation : 1;
		uint8 AbsoluteScale : 1;

		uint8 IsVisible : 1;
	public:

		Vector3 GetForwardVector() const;
		Vector3 GetUpVector() const;
		Vector3 GetRightVector() const;

		HSceneComponent* GetParent() const;

		const List<HSceneComponent*>& GetChildrens() const;
		void GetParentComponents(List<HSceneComponent*>& ParentComponent) const;

		uint32 GetNumChildrenComponents() const;
		HSceneComponent* GetChildComponent(uint32 index) const;

		bool AttachToComponent(HSceneComponent* InParent);
		void DetachFromComponent();

		bool IsVisible() const;

		virtual void OnVisibilityChanged();

		void SetVisibility(bool bNewVisibility, bool bPropagateToChildren = false);

		void ToggleVisibility(bool bPropagateToChildren = false)
		{
			SetVisibility(!IsVisible, bPropagateToChildren);
		}


	};
}