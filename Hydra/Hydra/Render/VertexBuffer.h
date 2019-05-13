#pragma once

#include "Hydra/Core/Vector.h"

struct VertexBufferEntry
{
	Vector3 Position;
	Vector2 TexCoord;
	Vector2 TexCoord2;
	Vector3 Color;
	Vector3 Normal;
	Vector3 Tangent;
	Vector3 BiTangent;
};