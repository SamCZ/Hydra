#include "WorldGeneratorActor.h"

#include "Hydra/EngineContext.h"

#include "Hydra/Render/Graphics.h"

#include "Hydra/Framework/Components/StaticMeshComponent.h"

#include "Hydra/Framework/Utils/ProceduralMesh.h"

#include "Game/World/Chunk.h"
#include "Game/World/BlockType.h"

#include "Hydra/Core/FastNoise.h"

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

void AddFace(ProceduralMesh& mesh, Block& block, float x, float y, float z, float pattern[4][3], const BlockFace& face)
{
	float size = 1.0f;
	float halfSize = size * 0.5f;

	Vector3 v0 = { x + halfSize + pattern[0][0] * halfSize, y + halfSize + pattern[0][1] * halfSize, z + halfSize + pattern[0][2] * halfSize };
	Vector3 v1 = { x + halfSize + pattern[1][0] * halfSize, y + halfSize + pattern[1][1] * halfSize, z + halfSize + pattern[1][2] * halfSize };
	Vector3 v2 = { x + halfSize + pattern[2][0] * halfSize, y + halfSize + pattern[2][1] * halfSize, z + halfSize + pattern[2][2] * halfSize };
	Vector3 v3 = { x + halfSize + pattern[3][0] * halfSize, y + halfSize + pattern[3][1] * halfSize, z + halfSize + pattern[3][2] * halfSize };

	Vector2 uv0 = { 0, 0 };
	Vector2 uv1 = { 0, 1 };
	Vector2 uv2 = { 1, 1 };
	Vector2 uv3 = { 1, 0 };

	Vector3 normal = FaceNormals[(int)face];

	mesh.AddVertex(v0, uv0, normal);
	mesh.AddVertex(v1, uv1, normal);
	mesh.AddVertex(v2, uv2, normal);
	mesh.AddVertex(v3, uv3, normal);

	mesh.AddQuadIndices();
}

HStaticMesh* CreateChunk()
{
	Chunk* chunk = new Chunk();

	BlockType* StoneType = new BlockType();

	FastNoise noise;
	noise.SetNoiseType(FastNoise::PerlinFractal);

	for (int x = 0; x < Chunk::ChunkWide; x++)
	{
		for (int z = 0; z < Chunk::ChunkDepth; z++)
		{
			for (int y = 0; y < Chunk::ChunkTall; y++)
			{
				float scale = 2.0f;

				float val = (noise.GetNoise(x * scale, y * scale, z * scale) + 1.0f) / 2.0f;
				//val *= 1.0f - pow((float)y / (float)Chunk::ChunkTall, 1);

				int state = round(val * (float)Chunk::ChunkTall);

				if (state > y)
				{
					chunk->SetBlock(x, y, z, Block(StoneType));
				}
			}
		}
	}

	ProceduralMesh mesh;

	for (int x = 0; x < Chunk::ChunkWide; x++)
	{
		for (int z = 0; z < Chunk::ChunkDepth; z++)
		{
			for (int y = 0; y < Chunk::ChunkTall; y++)
			{
				Block& block = chunk->GetBlock(x, y, z);

				if (IsAir(block))
				{
					continue;
				}

				Block& topBlock = chunk->GetBlock(x, y + 1, z);
				Block& bottomBlock = chunk->GetBlock(x, y - 1, z);
				Block& leftBlock = chunk->GetBlock(x - 1, y, z);
				Block& rightBlock = chunk->GetBlock(x + 1, y, z);
				Block& backBlock = chunk->GetBlock(x, y, z + 1);
				Block& frontBlock = chunk->GetBlock(x, y, z - 1);

				if (topBlock.Type != nullptr && topBlock.Type->IsTransparent)
				{
					if (topBlock.Type != block.Type || block.Type->ForceRenderAllFaces) topBlock = AirBlock;
				}

				if (bottomBlock.Type != nullptr && bottomBlock.Type->IsTransparent)
				{
					if (bottomBlock.Type != block.Type || block.Type->ForceRenderAllFaces) bottomBlock = AirBlock;
				}

				if (backBlock.Type != nullptr && backBlock.Type->IsTransparent)
				{
					if (backBlock.Type != block.Type || block.Type->ForceRenderAllFaces) backBlock = AirBlock;
				}

				if (frontBlock.Type != nullptr && frontBlock.Type->IsTransparent)
				{
					if (frontBlock.Type != block.Type || block.Type->ForceRenderAllFaces) frontBlock = AirBlock;
				}

				if (leftBlock.Type != nullptr && leftBlock.Type->IsTransparent)
				{
					if (leftBlock.Type != block.Type || block.Type->ForceRenderAllFaces) leftBlock = AirBlock;
				}

				if (rightBlock.Type != nullptr && rightBlock.Type->IsTransparent)
				{
					if (rightBlock.Type != block.Type || block.Type->ForceRenderAllFaces) rightBlock = AirBlock;
				}

				if (!IsAir(topBlock) && !IsAir(bottomBlock) && !IsAir(backBlock) && !IsAir(frontBlock) && !IsAir(leftBlock) && !IsAir(rightBlock))
				{
					continue;
				}

				if (IsAir(topBlock))
				{
					AddFace(mesh, block, x, y, z, TopBlockPattern, BlockFace::Top);
				}

				if (IsAir(bottomBlock))
				{
					AddFace(mesh, block, x, y, z, BottomBlockPattern, BlockFace::Bottom);
				}

				if (IsAir(leftBlock))
				{
					AddFace(mesh, block, x, y, z, LeftBlockPattern, BlockFace::Left);
				}

				if (IsAir(rightBlock))
				{
					AddFace(mesh, block, x, y, z, RightBlockPattern, BlockFace::Right);
				}

				if (IsAir(frontBlock))
				{
					AddFace(mesh, block, x, y, z, FrontBlockPattern, BlockFace::Front);
				}

				if (IsAir(backBlock))
				{
					AddFace(mesh, block, x, y, z, BackBlockPattern, BlockFace::Back);
				}
			}
		}
	}

	delete StoneType;
	delete chunk;

	return mesh.Create();
}

void WorldGeneratorActor::BeginPlay()
{


	HStaticMeshComponent* meshComponent = AddComponent<HStaticMeshComponent>("Mesh");
	meshComponent->StaticMesh = CreateChunk();

	FStaticMaterial staticMaterial = {  };
	staticMaterial.Material = Engine->GetAssetManager()->GetMaterial("Assets/Materials/World.mat");

	staticMaterial.Material->SetTexture("_GrassTex", Engine->GetAssetManager()->GetTexture("Assets/Textures/Terrain/Stony ground.png"));
	staticMaterial.Material->SetSampler("DefaultSampler", Engine->GetGraphics()->CreateSampler("Asdds"));

	meshComponent->StaticMesh->StaticMaterials.push_back(staticMaterial);

	Engine->GetAssetManager()->OnMeshLoaded.Invoke(meshComponent->StaticMesh);

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
		}
	}
}

void WorldGeneratorActor::Tick(float DeltaTime)
{
}
