#pragma once

#include "Object.h"

namespace Hydra
{
	struct FStaticMaterial
	{
		class MaterialInterface* Material;
	};

	class HStaticMesh : public HObject
	{
	public:

		class FStaticMeshRenderData* RenderData;
	};
}