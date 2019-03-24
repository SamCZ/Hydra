#include "Hydra/Terrain/Marching/MarchingCubes.h"
#include "Hydra/Terrain/Marching/MarchingTable.h"
#include "Hydra/Terrain/Marching/MarchingCubesTable.h"
#include "Hydra/Render/Mesh.h"

namespace Hydra
{
	using namespace MarchingTable;
	using namespace MarchingCubesTable;

	void MarchingCubes::March(float x, float y, float z, float* cube, Mesh* mesh)
	{
		int i, j, vert, idx;
		int flagIndex = 0;
		float offset = 0.0f;

		//Find which vertices are inside of the surface and which are outside
		for (i = 0; i < 8; i++) if (cube[i] <= _SurfaceThreshold) flagIndex |= 1 << i;

		//Find which edges are intersected by the surface
		int edgeFlags = CubeEdgeFlags[flagIndex];

		//If the cube is entirely inside or outside of the surface, then there will be no intersections
		if (edgeFlags == 0) return;

		//Find the point of intersection of the surface with each edge
		for (i = 0; i < 12; i++)
		{
			//if there is an intersection on this edge
			if ((edgeFlags & (1 << i)) != 0)
			{
				offset = GetOffset(cube[EdgeConnection[i][0]], cube[EdgeConnection[i][1]]);

				_EdgeVertex[i].x = x + (VertexOffset[EdgeConnection[i][0]][0] + offset * EdgeDirection[i][0]);
				_EdgeVertex[i].y = y + (VertexOffset[EdgeConnection[i][0]][1] + offset * EdgeDirection[i][1]);
				_EdgeVertex[i].z = z + (VertexOffset[EdgeConnection[i][0]][2] + offset * EdgeDirection[i][2]);
			}
		}

		//Save the triangles that were found. There can be up to five per cube
		for (i = 0; i < 5; i++)
		{
			if (TriangleConnectionTable[flagIndex, 3 * i] < 0) break;

			idx = (int)mesh->VertexData.size();

			for (j = 0; j < 3; j++)
			{
				vert = TriangleConnectionTable[flagIndex][3 * i + j];

				VertexBufferEntry entry = {};
				entry.position = _EdgeVertex[vert];
				
				mesh->VertexData.emplace_back(entry);

				unsigned int index = idx + WindingOrder[j];
				mesh->Indices.push_back(index);
			}
		}
	}
}