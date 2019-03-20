#include "Hydra/Scene/Transformable.h"

namespace Hydra
{
	static Vector3 axisX = Vector3(1, 0, 0);
	static Vector3 axisY = Vector3(0, 1, 0);
	static Vector3 axisZ = Vector3(0, 0, 1);

	Matrix4 Transformable::GetModelMatrix()
	{
		Matrix4 rotation = Matrix4();

		rotation *= glm::rotate(glm::radians(Rotation.z), axisZ);
		rotation *= glm::rotate(glm::radians(Rotation.y), axisY);
		rotation *= glm::rotate(glm::radians(Rotation.x), axisX);
		return (glm::translate(Position) * rotation) * glm::scale(Scale);

		/*rotation = glm::rotate(rotation, glm::radians(Rotation.z), axisZ);
		rotation = glm::rotate(rotation, glm::radians(Rotation.y), axisY);
		rotation = glm::rotate(rotation, glm::radians(Rotation.x), axisX);

		Matrix4 tranform = glm::translate(Position);
		
		tranform = glm::matrixCompMult(tranform, rotation);
		tranform = glm::matrixCompMult(tranform, glm::scale(Scale));

		return tranform;*/
	}

	Vector3 Transformable::GetRotationColumn(Matrix4& mat, int i)
	{
		Matrix4 trns = glm::transpose(mat);

		Vector3 store;

		store.x = trns[i][0];
		store.y = trns[i][1];
		store.z = trns[i][2];
		return store;
	}

	Vector3 Transformable::GetForward(Matrix4 & mat)
	{
		return GetRotationColumn(mat, 2);
	}

	Vector3 Transformable::GetLeft(Matrix4 & mat)
	{
		return GetRotationColumn(mat, 0);
	}

	Vector3 Transformable::GetUp(Matrix4 & mat)
	{
		return GetRotationColumn(mat, 1);
	}
}