#pragma once

#include "Hydra/Core/Vector.h"
#include "Hydra/Core/Container.h"

class BlockType
{
public:
	static Map<uint32_t, BlockType*> Types;
public:
	inline BlockType(uint32_t id) : ID(id), IsTransparent(false), ForceRenderAllFaces(false), HasMultipleTextureFaces(false)
	{
		Types[id] = this;
	}

	uint32_t ID;

	bool IsTransparent : 1;
	bool ForceRenderAllFaces : 1;

	Vector2 Texture[6];
	bool HasMultipleTextureFaces;
};