#pragma once

#include <stdint.h>

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