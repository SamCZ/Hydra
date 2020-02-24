#pragma once

#include "Hydra/Core/Vector.h"

class BlockType
{
public:
	uint32_t ID;

	bool IsTransparent : 1;
	bool ForceRenderAllFaces : 1;

	Vector2 Texture[6];
	bool HasMultipleTextureFaces;
};