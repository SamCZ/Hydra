#pragma once

#include "SceneComponent.h"
#include "CameraComponent.generated.h"


HCLASS()
class HYDRA_API HCameraComponent : public HSceneComponent
{
	HCLASS_GENERATED_BODY()
public:
	HCameraComponent();
	virtual ~HCameraComponent();
};
