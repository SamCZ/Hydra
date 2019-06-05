#pragma once

#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

struct FMeshBufferDataInternal
{
	NVRHI::BufferHandle VertexBuffer;
	NVRHI::BufferHandle IndexBuffer;

	FMeshBufferDataInternal() : VertexBuffer(nullptr), IndexBuffer(nullptr) {}
};