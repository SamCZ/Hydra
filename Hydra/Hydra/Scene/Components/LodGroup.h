#pragma once

#include "Hydra/Scene/Component.h"
#include "Hydra/Scene/Spatial.h"

namespace Hydra
{
	class LodGroup : public Component
	{
	private:
		List<SpatialPtr> _Lods;
	public:
		virtual void Start() override;
		virtual void Update() override;

	private:
		void SelectLod(int lod);
	};

	DEFINE_PTR(LodGroup)
}