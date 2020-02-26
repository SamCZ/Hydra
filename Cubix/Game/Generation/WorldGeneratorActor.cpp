#include "WorldGeneratorActor.h"

#include "Hydra/EngineContext.h"
#include "Hydra/Render/Graphics.h"
#include "Hydra/Framework/Components/StaticMeshComponent.h"
#include "Hydra/Framework/Utils/ProceduralMesh.h"

#include "Game/CubixGameMode.h"

#include "Game/World/World.h"

void WorldGeneratorActor::InitializeComponents()
{

}

static float TopBlockPattern[4][3]{
	{ -1, 1, -1 },
	{ -1, 1,  1 },
	{  1, 1,  1 },
	{  1, 1, -1 }
};

static float BottomBlockPattern[4][3]{
	{ -1, -1, -1 },
	{  1, -1, -1 },
	{  1, -1,  1 },
	{ -1, -1,  1 }
};

static float LeftBlockPattern[4][3]{
	{ -1,  1, -1 },
	{ -1, -1, -1 },
	{ -1, -1,  1 },
	{ -1,  1,  1 }
};

static float RightBlockPattern[4][3]{
	{  1,  1,  1 },
	{  1, -1,  1 },
	{  1, -1, -1 },
	{  1,  1, -1 }
};

static float FrontBlockPattern[4][3]{
	{  1,  1, -1 },
	{  1, -1, -1 },
	{ -1, -1, -1 },
	{ -1,  1, -1 }
};

static float BackBlockPattern[4][3]{
	{ -1,  1,  1 },
	{ -1, -1,  1 },
	{  1, -1,  1 },
	{  1,  1,  1 }
};

static Vector3 FaceNormals[6]{
	{ 0, 1, 0 },
	{ 0, -1, 0 },
	{ -1, 0, 0 },
	{ 1, 0, 0 },
	{ 0, 0, -1 },
	{ 0, 0, 1 }
};

void AddFace(ProceduralMesh& mesh, Chunk& chunk, Block& block, float x, float y, float z, float pattern[4][3], const BlockFace& face)
{
	Block* faceBlock = nullptr;

	switch (face)
	{
		case BlockFace::Top:
		faceBlock = &chunk.GetBlock((int)x, (int)y + 1, (int)z);
		break;
		case BlockFace::Bottom:
		faceBlock = &chunk.GetBlock((int)x, (int)y - 1, (int)z);
		break;
		case BlockFace::Left:
		faceBlock = &chunk.GetBlock((int)x - 1, (int)y, (int)z);
		break;
		case BlockFace::Right:
		faceBlock = &chunk.GetBlock((int)x + 1, (int)y, (int)z);
		break;
		case BlockFace::Front:
		faceBlock = &chunk.GetBlock((int)x, (int)y, (int)z - 1);
		break;
		case BlockFace::Back:
		faceBlock = &chunk.GetBlock((int)x, (int)y, (int)z + 1);
		break;
	}

	uint8_t lightLevel = 0;

	if (faceBlock != nullptr)
	{
		lightLevel = faceBlock->LightLevel;
	}

	float size = 1.0f;
	float halfSize = size * 0.5f;

	Vector3 v0 = { x + halfSize + pattern[0][0] * halfSize, y + halfSize + pattern[0][1] * halfSize, z + halfSize + pattern[0][2] * halfSize };
	Vector3 v1 = { x + halfSize + pattern[1][0] * halfSize, y + halfSize + pattern[1][1] * halfSize, z + halfSize + pattern[1][2] * halfSize };
	Vector3 v2 = { x + halfSize + pattern[2][0] * halfSize, y + halfSize + pattern[2][1] * halfSize, z + halfSize + pattern[2][2] * halfSize };
	Vector3 v3 = { x + halfSize + pattern[3][0] * halfSize, y + halfSize + pattern[3][1] * halfSize, z + halfSize + pattern[3][2] * halfSize };

	static float tUnit = 1.0F / (256.0f / 16.0f);

	Vector2& texture = block.Type->Texture[block.Type->HasMultipleTextureFaces ? (int)face : 0];

	Vector2 uv0 = { tUnit * texture.x, 1.0 - (tUnit * texture.y) };
	Vector2 uv1 = { tUnit * texture.x, 1.0 - (tUnit * texture.y + tUnit) };
	Vector2 uv2 = { tUnit * texture.x + tUnit, 1.0 - (tUnit * texture.y + tUnit) };
	Vector2 uv3 = { tUnit * texture.x + tUnit, 1.0 - (tUnit * texture.y) };

	Vector3 normal = FaceNormals[(int)face];

	mesh.AddVertex(v0, uv0, normal, Vector3(lightLevel, 0, 0));
	mesh.AddVertex(v1, uv1, normal, Vector3(lightLevel, 0, 0));
	mesh.AddVertex(v2, uv2, normal, Vector3(lightLevel, 0, 0));
	mesh.AddVertex(v3, uv3, normal, Vector3(lightLevel, 0, 0));

	mesh.AddQuadIndices();
}

HStaticMesh* CreateChunk(WorldPtr world, int chunkX, int chunkZ)
{
	LOG("Generating mesh...");
	Chunk* chunk = world->GetChunk(chunkX, chunkZ);

	ProceduralMesh mesh;

	for (int x = 0; x < Chunk::ChunkWide; x++)
	{
		for (int z = 0; z < Chunk::ChunkDepth; z++)
		{
			for (int y = 0; y < Chunk::ChunkTall; y++)
			{
				Block& block = world->GetBlock(chunkX, chunkZ, x, y, z);

				if (IsAir(block))
				{
					continue;
				}

				for (uint8_t i = 0; i < 6; i++)
				{
					const BlockFace face = (BlockFace)i;
					const Vector3i faceVector = GetFaceVector(face);


					Block& faceBlock = world->GetBlock(chunkX, chunkZ, x + faceVector.x, y + faceVector.y, z + faceVector.z);

					if (faceBlock.Type != nullptr && faceBlock.Type->IsTransparent)
					{
						if (faceBlock.Type != block.Type || block.Type->ForceRenderAllFaces) faceBlock = AirBlock;
					}

					if (IsAir(faceBlock))
					{
						switch (face)
						{
							case BlockFace::Top:
							AddFace(mesh, *chunk, block, x, y, z, TopBlockPattern, face);
							break;
							case BlockFace::Bottom:
							AddFace(mesh, *chunk, block, x, y, z, BottomBlockPattern, face);
							break;
							case BlockFace::Left:
							AddFace(mesh, *chunk, block, x, y, z, LeftBlockPattern, face);
							break;
							case BlockFace::Right:
							AddFace(mesh, *chunk, block, x, y, z, RightBlockPattern, face);
							break;
							case BlockFace::Front:
							AddFace(mesh, *chunk, block, x, y, z, FrontBlockPattern, face);
							break;
							case BlockFace::Back:
							AddFace(mesh, *chunk, block, x, y, z, BackBlockPattern, face);
							break;
						}
					}
				}

			}
		}
	}

	HStaticMesh* staticMesh = mesh.Create();

	LOG("Done generating mesh!");

	return staticMesh;
}

void WorldGeneratorActor::BeginPlay()
{
	CubixGameMode* gameMode = GetGameMode()->SafeCast<CubixGameMode>();

	FStaticMaterial staticMaterial = {  };
	staticMaterial.Material = Engine->GetAssetManager()->GetMaterial("Assets/Materials/World.mat");



	{
		HStaticMeshComponent* meshComponent = AddComponent<HStaticMeshComponent>("Mesh");
		meshComponent->StaticMesh = CreateChunk(gameMode->GetWorld(), 0, 0);
		staticMaterial.Material->SetTexture("_TerrainTexture", Engine->GetAssetManager()->GetTexture("Assets/Textures/terrain.png"));
		//staticMaterial.Material->SetSampler("_PixelSampler", Engine->GetGraphics()->CreateSampler("_PixelSampler", WrapMode::WRAP_MODE_BORDER, WrapMode::WRAP_MODE_BORDER, WrapMode::WRAP_MODE_BORDER, false, false, true));
		meshComponent->StaticMesh->StaticMaterials.push_back(staticMaterial);
		Engine->GetAssetManager()->OnMeshLoaded.Invoke(meshComponent->StaticMesh);
	}

	{
		HStaticMeshComponent* meshComponent = AddComponent<HStaticMeshComponent>("Mesh");
		meshComponent->StaticMesh = CreateChunk(gameMode->GetWorld(), 0, 1);
		staticMaterial.Material->SetTexture("_TerrainTexture", Engine->GetAssetManager()->GetTexture("Assets/Textures/terrain.png"));
		//staticMaterial.Material->SetSampler("_PixelSampler", Engine->GetGraphics()->CreateSampler("_PixelSampler", WrapMode::WRAP_MODE_BORDER, WrapMode::WRAP_MODE_BORDER, WrapMode::WRAP_MODE_BORDER, false, false, true));
		meshComponent->StaticMesh->StaticMaterials.push_back(staticMaterial);
		Engine->GetAssetManager()->OnMeshLoaded.Invoke(meshComponent->StaticMesh);
		meshComponent->Location.z = 16;
	}

	{
		HStaticMeshComponent* meshComponent = AddComponent<HStaticMeshComponent>("Mesh");
		meshComponent->StaticMesh = CreateChunk(gameMode->GetWorld(), 0, 2);
		staticMaterial.Material->SetTexture("_TerrainTexture", Engine->GetAssetManager()->GetTexture("Assets/Textures/terrain.png"));
		//staticMaterial.Material->SetSampler("_PixelSampler", Engine->GetGraphics()->CreateSampler("_PixelSampler", WrapMode::WRAP_MODE_BORDER, WrapMode::WRAP_MODE_BORDER, WrapMode::WRAP_MODE_BORDER, false, false, true));
		meshComponent->StaticMesh->StaticMaterials.push_back(staticMaterial);
		Engine->GetAssetManager()->OnMeshLoaded.Invoke(meshComponent->StaticMesh);
		meshComponent->Location.z = 16 * 2;
	}

	SetLocation(0, 0, -10);
}

void WorldGeneratorActor::BeginDestroy()
{
	for (HSceneComponent* component : Components)
	{
		if (component->IsA<HStaticMeshComponent>())
		{
			HStaticMeshComponent* meshComponent = component->SafeCast<HStaticMeshComponent>();

			Engine->GetAssetManager()->OnMeshDeleted.Invoke(meshComponent->StaticMesh);

			delete meshComponent->StaticMesh;
		}
	}
}

void WorldGeneratorActor::Tick(float DeltaTime)
{
}
