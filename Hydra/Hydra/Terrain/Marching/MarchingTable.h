#pragma once

namespace Hydra
{
	namespace MarchingTable
	{
		/// <summary>
		/// VertexOffset lists the positions, relative to vertex0, 
		/// of each of the 8 vertices of a cube.
		/// vertexOffset[8][3]
		/// </summary>
		static int VertexOffset[8][3]
		{
			{0, 0, 0},{1, 0, 0},{1, 1, 0},{0, 1, 0},
			{0, 0, 1},{1, 0, 1},{1, 1, 1},{0, 1, 1}
		};

		/// <summary>
		/// Winding order of triangles use 2,1,0 or 0,1,2
		/// </summary>
		static int WindingOrder[3]
		{ 
			0, 1, 2 
		};
	}
}