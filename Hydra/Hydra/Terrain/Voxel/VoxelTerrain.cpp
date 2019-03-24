#include "Hydra/Terrain/Voxel/VoxelTerrain.h"
#include "Hydra/Scene/Components/Camera.h"

#include "Hydra/Core/FastNoise.h"
#include "Hydra/Core/Random.h"
#include "Hydra/Terrain/Erosion.h"

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
					Vector2 p = offsets[i] + Vector2((x + xOff) * 0.05f, (y + yOff) * 0.05f) * scale;
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

		MarchingCubes mCubes;
		MarchingTertrahedron mTetrahedron;

		Marching* marching = &mCubes;

		Mesh* voxelMesh = marching->Generate(chunk.GetVoxelData(), 16, 16, 16);

		voxelMesh->SmoothMesh();
		voxelMesh->SmoothMesh();

		voxelMesh->GenerateNormals();
		voxelMesh->UpdateBuffers();

		MaterialPtr terrainMat = Material::CreateOrGet("Assets/Shaders/VoxelTerrain.hlsl");

		SpatialPtr chunkObj = MakeShared<Spatial>();

		chunkObj->Position = Vector3(pos.x * 14, -16, pos.y * 14);

		RendererPtr voxelRender = chunkObj->AddComponent<Renderer>();
		voxelRender->TestColor = MakeRGB(200, 200, 200).toVec3();
		voxelRender->Material = terrainMat;
		voxelRender->SetMesh(voxelMesh);

		return chunkObj;
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