#pragma once

namespace Hydra
{
	class Mesh;

	class Marching
	{
	private:
		float _Cube[8];
	protected:
		float _SurfaceThreshold;
	public:
		Marching(float surfaceThreshold = 0.5f);

		Mesh* Generate(float* voxels, int width, int height, int depth);

	protected:
		virtual void March(float x, float y, float z, float* cube, Mesh* mesh) = 0;

		virtual float GetOffset(float v1, float v2);
	};
}