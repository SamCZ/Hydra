#pragma once

#include "Hydra/Scene/Component.h"
#include "Hydra/Core/Vector.h"

namespace Hydra
{
	enum class CameraMode
	{
		Perspective,
		Orthographic
	};

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

		virtual void Start();
		virtual void Update();

		void SetCameraMode(const CameraMode& mode);
		CameraMode GetCameraMode;

		void SetFov(float fov);
		void SetZ(float zNear, float zFar);

		Matrix4 GetProjectionMatrix();
		Matrix4 GetViewMatrix();

		Matrix4 GetProjectionViewMatrix();

	private:
		void UpdateProjectionMatrix();
	};

	DEFINE_PTR(Camera)
}