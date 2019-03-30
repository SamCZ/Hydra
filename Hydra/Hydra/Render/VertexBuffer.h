#pragma once

#include "Hydra/Core/Vector.h"

namespace Hydra
{
	struct VertexBufferEntry
	{
		Vector3 position;
		Vector2 texCoord;
		Vector2 texCoord2;
		Vector3 normal;
		Vector3 tangent;
		Vector3 bitangent;
	};
}