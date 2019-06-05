#pragma once

#include "SceneComponent.h"
#include "Hydra/Render/View/ViewPort.h"

#include "Hydra/Physics/Collisons/Ray.h"

#include "CameraComponent.generated.h"

enum class FCameraMode
{
	Perspective,
	Orthographic
};

HCLASS()
class HYDRA_API HCameraComponent : public HSceneComponent
{
	HCLASS_GENERATED_BODY()
private:
	FCameraMode _LastCameraMode;

	Matrix4 _ProjectionMatrix;
	Matrix4 _ViewMatrix;
	Matrix4 _ProjectionViewMatrix;
public:
	FSceneView* SceneView;

	FCameraMode CameraMode;

	float Znear;
	float Zfar;

	float FOV;
public:
	HCameraComponent();
	virtual ~HCameraComponent();

	virtual void Tick(float Delta);

	Matrix4 GetProjectionMatrix() const;
	Matrix4 GetViewMatrix() const;
	Matrix4 GetProjectionViewMatrix() const;

	void UpdateProjectionMatrix();

public:
	Vector3 GetWorldPosition(float x, float y, float projectionZPos) const;
	Vector3 GetScreenCoordinates(const Vector3& position) const;

	Ray GetRay(float x, float y);
};
