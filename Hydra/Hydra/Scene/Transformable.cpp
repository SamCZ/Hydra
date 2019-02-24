#include "Hydra/Scene/Transformable.h"

namespace Hydra
{
	Matrix4 Transformable::GetModelMatrix()
	{
		Matrix4 rotation = Matrix4();

		static Vector3 axisX = Vector3(1, 0, 0);
		static Vector3 axisY = Vector3(0, 1, 0);
		static Vector3 axisZ = Vector3(0, 0, 1);

		rotation *= glm::rotate(glm::radians(Rotation.z), axisZ);
		rotation *= glm::rotate(glm::radians(Rotation.y), axisY);
		rotation *= glm::rotate(glm::radians(Rotation.x), axisX);

		return (glm::translate(Position) * rotation) * glm::scale(Scale);
	}
}