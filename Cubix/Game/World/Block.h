#pragma once

#include <stdint.h>
#include "Hydra/Core/Vector.h"

class BlockType;

enum class BlockFace : uint8_t
{
	Top = 0,
	Bottom = 1,
	Left = 2,
	Right = 3,
	Front = 4,
	Back = 5
};

static Vector3i GetFaceVector(const BlockFace& face)
{
	switch (face)
	{
		case BlockFace::Top:
		return Vector3i(0, 1, 0);
		case BlockFace::Bottom:
		return Vector3i(0, -1, 0);
		case BlockFace::Left:
		return Vector3i(-1, 0, 0);
		case BlockFace::Right:
		return Vector3i(1, 0, 0);
		case BlockFace::Front:
		return Vector3i(0, 0, -1);
		case BlockFace::Back:
		return Vector3i(0, 0, 1);
		default:
		return Vector3i(0, 0, 0);
	}
}

struct Block
{
	BlockType* Type;

	uint8_t Meta = 0;

	uint8_t LightLevel = 0;
	uint8_t LightStage = 0;

public:
	explicit constexpr Block(BlockType* type) : Type(type)
	{
	}
	constexpr Block() : Block(nullptr)
	{
	}
};

static inline bool IsAir(const Block& block)
{
	return block.Type == nullptr;
}

static Block AirBlock;