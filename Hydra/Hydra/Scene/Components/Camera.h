#pragma once

#include "Hydra/Scene/Component.h"
#include "Hydra/Core/Vector.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Physics/Collisons/Ray.h"

namespace Hydra
{
	enum class CameraMode
	{
		Perspective,
		Orthographic
	};

	class Camera;

	DEFINE_PTR(Camera)

	class Camera : public Component
	{
	private:
		CameraMode _CameraMode;
		float _FovY;
		float _Znear;
		float _Zfar;

		int _Width;
		int _Height;

		float _FrustumLeft;
		float _FrustumRight;
		float _FrustumBottom;
		float _FrustumTop;
		float _FrustumNear;
		float _FrustumFar;

		Matrix4 _ProjectionMatrix;
		Matrix4 _ViewMatrix;
		Matrix4 _ViewProjectionMatrix;
	public:
		Camera();

		static CameraPtr MainCamera;
		static List<CameraPtr> AllCameras;

		virtual void Start();
		virtual void Update();

		void SetCameraMode(const CameraMode& mode);
		CameraMode GetCameraMode();

		void Resize(int width, int height);

		void SetFov(float fov);
		void SetZ(float zNear, float zFar);

		Vector3 GetWorldPosition(float x, float y, float projectionZPos) const;
		Vector3 GetScreenCoordinates(const Vector3& pos);

		Ray GetRay(int x, int y);

		Matrix4 GetProjectionMatrix();
		Matrix4 GetViewMatrix();

		Matrix4 GetProjectionViewMatrix();

		Vector3 GetForward();
		Vector3 GetLeft();
		Vector3 GetUp();

		float GetZNear();
		float GetZFar();

		int GetWidth();
		int GetHeight();

	private:
		void UpdateProjectionMatrix();
	};
}