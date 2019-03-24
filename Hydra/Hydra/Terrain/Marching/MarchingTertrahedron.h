#pragma once

#include "Hydra/Terrain/Marching/Marching.h"
#include "Hydra/Core/Vector.h"

namespace Hydra
{
	class MarchingTertrahedron : public Marching
	{
	private:
		Vector3 _EdgeVertex[6];
		Vector3 _CubePosition[8];
		Vector3 _TetrahedronPosition[4];
		float _TetrahedronValue[4];
	protected:
		virtual void March(float x, float y, float z, float* cube, Mesh* mesh);
		void MarchTetrahedron(Mesh* mesh);
	};
}