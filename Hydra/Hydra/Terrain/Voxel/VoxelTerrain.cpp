#include "Hydra/Terrain/Voxel/VoxelTerrain.h"
#include "Hydra/Scene/Components/Camera.h"

#include "Hydra/Core/FastNoise.h"
#include "Hydra/Core/Random.h"
#include "Hydra/Terrain/Erosion.h"

#include "Hydra/Terrain/Generator/MeshBuilder.h"

#include "Hydra/Terrain/Marching/MarchingCubes.h"
#include "Hydra/Terrain/Marching/MarchingTertrahedron.h"

#include "Hydra/Render/Material.h"
#include "Hydra/Scene/Components/Renderer.h"
#include "Hydra/Core/ColorRGBA.h"

#include "Hydra/Render/Mesh.h"

namespace Hydra
{
	VoxelTerrain::VoxelTerrain() : Spatial("VoxelTerrain")
	{

	}

	VoxelTerrain::~VoxelTerrain()
	{

	}

	void VoxelTerrain::Start()
	{

	}

	enum BlockFace
	{
		Front,
		Back,
		Left,
		Right,
		Top,
		Bottom
	};

	void AddFace(MeshBuilder& builder, const BlockFace& face, const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& v3)
	{
		builder.AddQuad(v0, v1, v2, v3, Vector2(), Vector2(), Vector2(), Vector2());
	}

	inline float* GenerateNoiseMapYo(int mapSize, float xOff, float yOff)
	{
		int seed = 0;
		bool randomizeSeed = true;

		int numOctaves = 7;
		float persistence = .5f;
		float lacunarity = 2;
		float initialScale = 100.0f;

		auto map = new float[mapSize * mapSize];
		auto prng = Random(seed);

		Vector2* offsets = new Vector2[numOctaves];
		for (int i = 0; i < numOctaves; i++)
		{
			offsets[i] = Vector2(prng.GetFloat(-1000, 1000), prng.GetFloat(-1000, 1000));
		}

		float minValue = 999999.999f;
		float maxValue = -999999.999f;

		FastNoise noise;
		noise.SetNoiseType(FastNoise::NoiseType::Perlin);

		for (int y = 0; y < mapSize; y++)
		{
			for (int x = 0; x < mapSize; x++)
			{
				float noiseValue = 0;
				float scale = initialScale;
				float weight = 1;
				for (int i = 0; i < numOctaves; i++)
				{
					Vector2 p = offsets[i] + Vector2((x + xOff) * 0.01f, (y + yOff) * 0.01f) * scale;
					noiseValue += noise.GetNoise(p.x, p.y) * weight;
					weight *= persistence;
					scale *= lacunarity;
				}
				map[y * mapSize + x] = noiseValue;
				minValue = std::min(noiseValue, minValue);
				maxValue = std::max(noiseValue, maxValue);
			}
		}

		delete[] offsets;

		// Normalize
		/*if (maxValue != minValue)
		{
			for (int i = 0; i < mapSize * mapSize; i++)
			{
				map[i] = (map[i] - minValue) / (maxValue - minValue);
			}
		}*/

		return map;
	}

	SpatialPtr CreateVoxel(Vector2i pos)
	{
		SpatialPtr spatial = MakeShared<Spatial>();
		spatial->Position = Vector3(pos.x * 16, -16, pos.y * 16);

		float* noiseMap = GenerateNoiseMapYo(16, pos.x * 16, pos.y * 16);

		Erosion erosion;
		//erosion.Erode(noiseMap, 16, 500);

		VoxelChunk chunk(16, 16, 16);

		for (int x = 0; x < 16; x++)
		{
			for (int y = 0; y < 16; y++)
			{
				for (int z = 0; z < 16; z++)
				{
					float noiseVal = noiseMap[x + z * 16] * 2.0f + 1.0;
					noiseVal *= 5;

					//std::cout << noiseVal << std::endl;

					if (noiseVal > y)
					{
						chunk.SetVoxel(x, y, z, 1);
					}
					else
					{
						chunk.SetVoxel(x, y, z, 0);
					}
				}
			}
		}

		delete[] noiseMap;

		int topBlock = 0;
		int bottomBlock = 0;
		int leftBlock = 0;
		int rightBlock = 0;
		int backBlock = 0;
		int frontBlock = 0;

		MeshBuilder builder;

		for (int x = 0; x < 16; x++)
		{
			for (int z = 0; z < 16; z++)
			{
				for (int y = 0; y < 16; y++)
				{
					int blockId = chunk.GetVoxel(x, y, z);

					if (blockId > 0)
					{
						topBlock = chunk.GetVoxel(x, y + 1, z);
						bottomBlock = chunk.GetVoxel(x, y - 1, z);
						leftBlock = chunk.GetVoxel(x - 1, y, z);
						rightBlock = chunk.GetVoxel(x + 1, y, z);
						backBlock = chunk.GetVoxel(x, y, z + 1);
						frontBlock = chunk.GetVoxel(x, y, z - 1);

						if (topBlock != blockId) topBlock = 0;

						if (bottomBlock != blockId) bottomBlock = 0;
						if (backBlock != blockId) backBlock = 0;
						if (frontBlock != blockId) frontBlock = 0;
						if (leftBlock != blockId) leftBlock = 0;
						if (rightBlock != blockId) rightBlock = 0;

						if (topBlock > 0 && bottomBlock > 0 && backBlock > 0 && frontBlock > 0 && leftBlock > 0 && rightBlock > 0)
						{
							continue;
						}

						float microspace = 0.00001f;

						float size = 1.0f + microspace;
						float sizeY = 1.0f + microspace;

						if (topBlock == 0)
						{
							AddFace(builder, BlockFace::Top,
								{ x, y + sizeY, z },
								{ x, y + sizeY, z + size },
								{ x + size, y + sizeY, z + size },
								{ x + size, y + sizeY, z });
						}

						if (bottomBlock == 0)
						{
							AddFace(builder, BlockFace::Bottom,
								{ x, y, z },
								{ x + sizeY, y, z },
								{ x + sizeY, y, z + size },
								{ x, y, z + size });
						}

						if (leftBlock == 0)
						{
							AddFace(builder, BlockFace::Left,
								{ x, y + sizeY, z },
								{ x, y, z },
								{ x, y, z + size },
								{ x, y + sizeY, z + size });
						}

						if (rightBlock == 0)
						{
							AddFace(builder, BlockFace::Right,
								{ x + size, y + sizeY, z + size },
								{ x + size, y, z + size },
								{ x + size, y, z },
								{ x + size, y + sizeY, z });
						}

						if (frontBlock == 0)
						{
							AddFace(builder, BlockFace::Front,
								{ x + size, y + sizeY, z },
								{ x + size, y, z },
								{ x, y, z },
								{ x, y + sizeY, z });
						}

						if (backBlock == 0)
						{
							AddFace(builder, BlockFace::Back,
								{ x, y + sizeY, z + size },
								{ x, y, z + size },
								{ x + size, y, z + size },
								{ x + size, y + sizeY, z + size });
						}
					}
				}
			}
		}

		builder.Apply(spatial);

		return spatial;
	}

	void VoxelTerrain::Update()
	{
		Vector3 camLocation = Camera::MainCamera->GameObject->Position;
		int chunkX = (int)glm::ceil(camLocation.x / 16.0f);
		int chunkZ = (int)glm::ceil(camLocation.z / 16.0f);

		int drawDistance = 5;

		for (int cx = chunkX - drawDistance; cx < chunkX + drawDistance; cx++)
		{
			for (int cz = chunkZ - drawDistance; cz < chunkZ + drawDistance; cz++)
			{
				Vector2i chunkPos = Vector2(cx, cz);

				if (Hydra::Find(_Chunks, chunkPos) == _Chunks.end())
				{
					
					std::cout << glm::to_string(chunkPos) << std::endl;
					AddChild(CreateVoxel(chunkPos));


					_Chunks.push_back(chunkPos);
				}
			}
		}


	}
}