#include "Hydra/Scene/Components/Camera.h"
#include "Hydra/Scene/Spatial.h"

#include <iostream>

#include "Hydra/Engine.h"

namespace Hydra
{
	Camera::Camera() : _FovY(75.0f), _Znear(0.01f), _Zfar(1000.0f)
	{
		
	}

	void Camera::Start()
	{
		_Width = 1280;
		_Height = 720;
		SetCameraMode(CameraMode::Perspective);
	}

	void Camera::Update()
	{
		Vector3 location = Parent->Position;
		Vector3 rotation = Parent->Rotation;

		_ViewMatrix = glm::mat4();
		// define your up vector
		glm::vec3 upVector = glm::vec3(0, 1, 0);
		// rotate around to a given bearing: yaw
		glm::mat4 camera = glm::rotate(glm::mat4(), glm::radians(rotation.y), upVector);
		// Define the 'look up' axis, should be orthogonal to the up axis
		glm::vec3 pitchVector = glm::vec3(1, 0, 0);
		// rotate around to the required head tilt: pitch
		camera = glm::rotate(camera, glm::radians(rotation.x), pitchVector);

		glm::vec3 rollVector = glm::vec3(0, 0, 1);
		camera = glm::rotate(camera, glm::radians(rotation.z), rollVector);

		// now get the view matrix by taking the camera inverse
		_ViewMatrix = glm::inverse(camera) * glm::translate(_ViewMatrix, glm::vec3(-location.x, -location.y, -location.z));

		_ViewProjectionMatrix = _ProjectionMatrix * _ViewMatrix;
	}

	void Camera::SetCameraMode(const CameraMode & mode)
	{
		_CameraMode = mode;
		UpdateProjectionMatrix();
	}

	void Camera::SetFov(float fov)
	{
		_FovY = fov;
	}

	void Camera::SetZ(float zNear, float zFar)
	{
		_Znear = zNear;
		_Zfar = zFar;
	}

	Matrix4 Camera::GetProjectionMatrix()
	{
		return _ProjectionMatrix;
	}

	Matrix4 Camera::GetViewMatrix()
	{
		return _ViewMatrix;
	}

	Matrix4 Camera::GetProjectionViewMatrix()
	{
		return _ViewProjectionMatrix;
	}

	void Camera::UpdateProjectionMatrix()
	{
		if (_CameraMode == CameraMode::Perspective)
		{
			float aspect = (float)_Width / (float)_Height;
			float h = glm::tan(glm::radians(_FovY) * .5f) * _Znear;
			float w = h * aspect;
			_FrustumLeft = -w;
			_FrustumRight = w;
			_FrustumBottom = -h;
			_FrustumTop = h;
			_FrustumNear = _Znear;
			_FrustumFar = _Zfar;

			_ProjectionMatrix = glm::frustum(_FrustumLeft, _FrustumRight, _FrustumBottom, _FrustumTop, _FrustumNear, _FrustumFar);
		}
		else if (_CameraMode == CameraMode::Orthographic)
		{
			_ProjectionMatrix = glm::ortho(0.0f, (float)_Width, (float)_Height, 0.0f, -1.0f, 1.0f);
		}
	}
}