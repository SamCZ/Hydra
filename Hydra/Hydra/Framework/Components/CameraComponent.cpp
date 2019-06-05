#include "CameraComponent.h"

HCameraComponent::HCameraComponent() : HSceneComponent(), SceneView(nullptr), _LastCameraMode(FCameraMode::Orthographic), CameraMode(FCameraMode::Perspective), Znear(0.01f), Zfar(2000.0f), FOV(75.0f)
{

}

HCameraComponent::~HCameraComponent()
{
}

void HCameraComponent::Tick(float Delta)
{
	if (SceneView == nullptr)
	{
		return;
	}

	if (_LastCameraMode != CameraMode)
	{
		_LastCameraMode = CameraMode;

		UpdateProjectionMatrix();
	}

	UpdateProjectionMatrix();

	_ViewMatrix = GetTransformMatrix();


	_ProjectionViewMatrix = GetProjectionMatrix() * GetViewMatrix();
}

Matrix4 HCameraComponent::GetProjectionMatrix() const
{
	return _ProjectionMatrix;
}

Matrix4 HCameraComponent::GetViewMatrix() const
{
	return _ViewMatrix;
}

Matrix4 HCameraComponent::GetProjectionViewMatrix() const
{
	return _ProjectionViewMatrix;
}

void HCameraComponent::UpdateProjectionMatrix()
{
	if (SceneView == nullptr)
	{
		return;
	}

	if (CameraMode == FCameraMode::Perspective)
	{
		float aspect = (float)SceneView->Width / (float)SceneView->Height;
		float h = glm::tan(glm::radians(FOV) * .5f) * Znear;
		float w = h * aspect;
		float _FrustumLeft = -w;
		float _FrustumRight = w;
		float _FrustumBottom = -h;
		float _FrustumTop = h;
		float _FrustumNear = Znear;
		float _FrustumFar = Zfar;

		_ProjectionMatrix = glm::frustum(_FrustumLeft, _FrustumRight, _FrustumBottom, _FrustumTop, _FrustumNear, _FrustumFar);
	}
	else
	{
		_ProjectionMatrix = glm::ortho(0.0f, (float)SceneView->Width, (float)SceneView->Height, 0.0f, -1.0f, 1.0f);
	}
}

Vector3 HCameraComponent::GetWorldPosition(float x, float y, float projectionZPos) const
{
	Matrix4 inverseMat = glm::inverse(GetProjectionViewMatrix());

	Vector3 store = Vector3((2.0f * x) / SceneView->Width - 1.0f, (2.0f * y) / SceneView->Height - 1.0f, projectionZPos * 2 - 1);
	Vector4 proStore = inverseMat * Vector4(store, 1.0f);
	store.x = proStore.x;
	store.y = proStore.y;
	store.z = proStore.z;
	store *= 1.0f / proStore.w;
	return store;
}

Vector3 HCameraComponent::GetScreenCoordinates(const Vector3 & position) const
{
	Vector4 proj = (GetProjectionViewMatrix() * Vector4(position, 1.0f));

	Vector3 store = Vector3(proj.x, proj.y, proj.z);
	store = store / proj.w;

	int viewPortRight = 1;
	int viewPortLeft = 0;
	int viewPortTop = 0;
	int viewPortBottom = 1;

	store.x = ((store.x + 1.0f) * (viewPortRight - viewPortLeft) / 2.0f + viewPortLeft) * SceneView->Width;
	store.y = ((store.y + 1.0f) * (viewPortTop - viewPortBottom) / 2.0f + viewPortBottom) * SceneView->Height;
	store.z = (store.z + 1.0f) / 2.0f;

	return store;
}

Ray HCameraComponent::GetRay(float x, float y)
{
	Vector3 click3d = GetWorldPosition(x, y, 0);
	Vector3 dir = glm::normalize(GetWorldPosition(x, y, 1) - click3d);
	return Ray(click3d, dir);
}
