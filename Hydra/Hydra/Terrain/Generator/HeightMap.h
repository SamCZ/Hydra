#pragma once

namespace Hydra
{
	struct HeightMap
	{
		int Width;
		int Height;
		float* Data;

		~HeightMap()
		{
			delete[] Data;
		}
	};
}