#include "Hydra/Scene/Transformable.h"

#include <glm/basic.hpp>

namespace Hydra
{
	Matrix4 Transformable::GetModelMatrix()
	{
		glm::mat4 rotation = glm::mat4();

		glm::vec3 axisX = glm::vec3(1, 0, 0);
		glm::vec3 axisY = glm::vec3(0, 1, 0);
		glm::vec3 axisZ = glm::vec3(0, 0, 1);

		rotation *= glm::rotate(glm::radians(Rotation.z), axisZ);
		rotation *= glm::rotate(glm::radians(Rotation.y), axisY);
		rotation *= glm::rotate(glm::radians(Rotation.x), axisX);

		return (glm::translate(Position) * rotation) * glm::scale(Scale);
	}
}