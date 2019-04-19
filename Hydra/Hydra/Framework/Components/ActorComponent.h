#pragma once

#include "Hydra/Framework/Object.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/String.h"

class AActor;
class FWorld;
class EngineContext;

class HYDRA_API HActorComponent : public HObject
{
	HCLASS_BODY(HActorComponent)
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