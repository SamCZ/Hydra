#pragma once

#include "Hydra/Terrain/Marching/Marching.h"
#include "Hydra/Core/Vector.h"

namespace Hydra
{
	class MarchingCubes : public Marching
	{
	private:
		Vector3 _EdgeVertex[12];
	protected:
		virtual void March(float x, float y, float z, float* cube, Mesh* mesh);
	};
}