#pragma once

class BlockType
{
public:
	uint32_t ID;

	bool IsTransparent : 1;
	bool ForceRenderAllFaces : 1;
};