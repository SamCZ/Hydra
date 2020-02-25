#pragma once

#include "Game/World/Block.h"

class IBlockAccess
{
public:
	virtual Block& GetBlock(int x, int y, int z) = 0;
	virtual void SetBlock(int x, int y, int z, const Block& block) = 0;
};