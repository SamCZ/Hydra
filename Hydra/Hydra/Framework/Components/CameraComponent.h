#pragma once

#include "SceneComponent.h"
#include "Hydra/Render/View/SceneView.h"

#include "CameraComponent.generated.h"

HCLASS()
class HYDRA_API HCameraComponent : public HSceneComponent
{
	HCLASS_GENERATED_BODY()
public:
	FSceneView* SceneView;
public:
	HCameraComponent();
	virtual ~HCameraComponent();
};
