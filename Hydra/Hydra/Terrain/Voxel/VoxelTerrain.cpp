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
		auto map = new float[mapSize * mapSize];
		FastNoise noise;
		noise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);

		float scale = 1.0;

		for (int y = 0; y < mapSize; y++)
		{
			for (int x = 0; x < mapSize; x++)
			{
				float noiseValue = 0.0f;

				float xx = x + xOff;
				float yy = y + yOff;

				noiseValue += noise.GetNoise(xx * scale, yy * scale);
				
				if (noise.GetNoise(xx * 0.1f, yy * 0.1f) > 0.0)
				{
					noiseValue *= 5.0f;
				}

				map[y * mapSize + x] = noiseValue;
			}
		}


		return map;
	}

	SpatialPtr CreateVoxel(Vector2i pos)
	{
		SpatialPtr spatial = MakeShared<Spatial>();
		spatial->Position = Vector3(pos.x * 16, -16, pos.y * 16);

		float* noiseMap = GenerateNoiseMapYo(16, pos.x * 15, pos.y * 15);

		Erosion erosion;
		//erosion.Erode(noiseMap, 16, 500);

		VoxelChunk chunk(16, 64, 16);

		for (int x = 0; x < 16; x++)
		{
			for (int y = 0; y < 64; y++)
			{
				for (int z = 0; z < 16; z++)
				{
					if (y == 0)
					{
						chunk.SetVoxel(x, y, z, 1);
						continue;
					}

					float scale = 0.1f;
					float noiseVal = noiseMap[x + z * 16] * 2.0f + 1.0;

					/*float xx = x + pos.x * 15;
					float zz = z + pos.y * 15;

					float noiseVal = glm::cos(xx * scale) * glm::sin(zz * scale);
					noiseVal = noiseVal * 0.5f + 0.5f;*/
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

		MarchingCubes mCubes;
		MarchingTertrahedron mTetrahedron;

		Marching* marching = &mCubes;

		Mesh* voxelMesh = marching->Generate(chunk.GetVoxelData(), 16, 64, 16);

		//voxelMesh->SmoothMesh();
		//voxelMesh->SmoothMesh();

		voxelMesh->GenerateNormals();
		voxelMesh->UpdateBuffers();

		MaterialPtr terrainMat = Material::CreateOrGet("Assets/Shaders/VoxelTerrain.hlsl");

		SpatialPtr chunkObj = MakeShared<Spatial>();

		chunkObj->Position = Vector3(pos.x * 15, -16, pos.y * 15);

		RendererPtr voxelRender = chunkObj->AddComponent<Renderer>();
		voxelRender->TestColor = MakeRGB(200, 200, 200).toVec3();
		voxelRender->Material = terrainMat;
		voxelRender->SetMesh(voxelMesh);

		return chunkObj;

		/*int topBlock = 0;
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

		return spatial;*/
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
					AddChild(CreateVoxel(chunkPos));
					_Chunks.push_back(chunkPos);
				}
			}
		}


	}
}