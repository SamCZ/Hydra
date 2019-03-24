#include "Hydra/Terrain/Marching/MarchingTertrahedron.h"
#include "Hydra/Terrain/Marching/MarchingTable.h"
#include "Hydra/Terrain/Marching/MarchingTertrahedronTable.h"
#include "Hydra/Render/Mesh.h"

namespace Hydra
{
	using namespace MarchingTable;
	using namespace MarchingTertrahedronTable;

	void MarchingTertrahedron::March(float x, float y, float z, float* cube, Mesh* mesh)
	{
		int i, j, vertexInACube;

		//Make a local copy of the cube's corner positions
		for (i = 0; i < 8; i++)
		{
			_CubePosition[i].x = x + VertexOffset[i][0];
			_CubePosition[i].y = y + VertexOffset[i][1];
			_CubePosition[i].z = z + VertexOffset[i][2];
		}

		for (i = 0; i < 6; i++)
		{
			for (j = 0; j < 4; j++)
			{
				vertexInACube = TetrahedronsInACube[i][j];
				_TetrahedronPosition[j] = _CubePosition[vertexInACube];
				_TetrahedronValue[j] = cube[vertexInACube];
			}

			MarchTetrahedron(mesh);
		}
	}

	void MarchingTertrahedron::MarchTetrahedron(Mesh* mesh)
	{
		int i, j, vert, vert0, vert1, idx;
		int flagIndex = 0, edgeFlags;
		float offset, invOffset;

		//Find which vertices are inside of the surface and which are outside
		for (i = 0; i < 4; i++) if (_TetrahedronValue[i] <= _SurfaceThreshold) flagIndex |= 1 << i;

		//Find which edges are intersected by the surface
		edgeFlags = TetrahedronEdgeFlags[flagIndex];

		//If the tetrahedron is entirely inside or outside of the surface, then there will be no intersections
		if (edgeFlags == 0) return;

		//Find the point of intersection of the surface with each edge
		for (i = 0; i < 6; i++)
		{
			//if there is an intersection on this edge
			if ((edgeFlags & (1 << i)) != 0)
			{
				vert0 = TetrahedronEdgeConnection[i][0];
				vert1 = TetrahedronEdgeConnection[i][1];
				offset = GetOffset(_TetrahedronValue[vert0], _TetrahedronValue[vert1]);
				invOffset = 1.0f - offset;

				_EdgeVertex[i].x = invOffset * _TetrahedronPosition[vert0].x + offset * _TetrahedronPosition[vert1].x;
				_EdgeVertex[i].y = invOffset * _TetrahedronPosition[vert0].y + offset * _TetrahedronPosition[vert1].y;
				_EdgeVertex[i].z = invOffset * _TetrahedronPosition[vert0].z + offset * _TetrahedronPosition[vert1].z;
			}
		}

		//Save the triangles that were found. There can be up to 2 per tetrahedron
		for (i = 0; i < 2; i++)
		{
			if (TetrahedronTriangles[flagIndex, 3 * i] < 0) break;

			idx = (int)mesh->VertexData.size();

			for (j = 0; j < 3; j++)
			{
				vert = TetrahedronTriangles[flagIndex][3 * i + j];

				VertexBufferEntry entry = {};
				entry.position = _EdgeVertex[vert];
				mesh->VertexData.emplace_back(entry);

				unsigned int index = idx + WindingOrder[j];
				mesh->Indices.push_back(index);
			}
		}
	}
}