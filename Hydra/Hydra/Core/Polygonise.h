#pragma once

#include "Hydra/Core/Vector.h"

namespace Hydra
{

	typedef struct
	{
		Vector3 p[3];
	} TRIANGLE;

	typedef struct
	{
		Vector3 p[8];
		double val[8];
	} GRIDCELL;

	/*
	 Given a grid cell and an isolevel, calculate the triangular
	 facets required to represent the isosurface through the cell.
	 Return the number of triangular facets, the array "triangles"
	 will be loaded up with the vertices at most 5 triangular facets.
	 0 will be returned if the grid cell is either totally above
	 of totally below the isolevel.
	 */
	int Polygonise(GRIDCELL& grid, double isolevel, TRIANGLE *triangles);
	Vector3 VertexInterp(double isolevel, const Vector3& p1, const Vector3& p2, double valp1, double valp2);
}