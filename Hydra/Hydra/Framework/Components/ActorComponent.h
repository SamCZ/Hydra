#pragma once

#include "Hydra/Framework/Object.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/String.h"
#include "ActorComponent.generated.h"


class AActor;
class FWorld;
class EngineContext;

HCLASS()
class HYDRA_API HActorComponent : public HObject
{
	HCLASS_GENERATED_BODY()
private:
	uint8 RenderTransformDirty : 1;
public:
	List<String> Tags;

	uint8 IsActive : 1;
	uint8 IsEditorOnly : 1;

	FWorld* World;
	EngineContext* Engine;
	AActor* Owner;
public:
	HActorComponent();

	virtual void SetActive(bool newActive);
	virtual void ToggleActive();

	virtual void MarkAsEditorOnlySubobject()
	{
		IsEditorOnly = true;
	}

	virtual void BeginPlay();
	virtual void BeginDestroy();
};
