#pragma once

#include "Hydra/Scene/Spatial.h"

namespace Hydra
{
	class Material;

	class Terrain : public Spatial
	{
	private:

	public:
		Terrain(SharedPtr<Material> material);
		~Terrain();

		virtual void Start();
		virtual void Update();

	};

	DEFINE_PTR(Terrain)
}