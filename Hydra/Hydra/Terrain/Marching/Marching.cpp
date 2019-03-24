#include "Hydra/Terrain/Marching/Marching.h"
#include "Hydra/Terrain/Marching/MarchingTable.h"
#include "Hydra/Render/Mesh.h"
#include "Hydra/Core/Log.h"

namespace Hydra
{
	using namespace MarchingTable;

	Marching::Marching(float surfaceThreshold) : _SurfaceThreshold(surfaceThreshold)
	{

	}

	Mesh* Marching::Generate(int* voxels, int width, int height, int depth)
	{
		if (_SurfaceThreshold > 0.0f)
		{
			WindingOrder[0] = 0;
			WindingOrder[1] = 1;
			WindingOrder[2] = 2;
		}
		else
		{
			WindingOrder[0] = 2;
			WindingOrder[1] = 1;
			WindingOrder[2] = 0;
		}

		Mesh* mesh = new Mesh();

		int x, y, z, i;
		int ix, iy, iz;
		for (x = 0; x < width - 1; x++)
		{
			for (y = 0; y < height - 1; y++)
			{
				for (z = 0; z < depth - 1; z++)
				{
					//Get the values in the 8 neighbours which make up a cube
					for (i = 0; i < 8; i++)
					{
						ix = x + VertexOffset[i][0];
						iy = y + VertexOffset[i][1];
						iz = z + VertexOffset[i][2];

						_Cube[i] = voxels[ix + iy * width + iz * width * height];
					}

					//Perform algorithm
					March((float)x, (float)y, (float)z, _Cube, mesh);
				}
			}
		}

		Log("Marching::Generate", ToString(width) + ", " + ToString(height) + ", " + ToString(depth), "Generated (" + ToString(mesh->VertexData.size()) + ")");

		return mesh;
	}

	float Marching::GetOffset(float v1, float v2)
	{
		float delta = v2 - v1;
		return (delta == 0.0f) ? _SurfaceThreshold : (_SurfaceThreshold - v1) / delta;
	}
}