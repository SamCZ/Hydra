#pragma once

#include "Hydra/Core/Vector.h"

namespace Hydra
{
	struct VertexBufferEntry
	{
		Vector3 position;
		Vector2 texCoord;
		Vector3 normal;
		Vector3 tangent;
		Vector3 binormal;
	};
}