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

	float tUnit = 1.0F / (256.0f / 16.0f);

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

HStaticMesh* CreateChunk()
{
	Chunk* chunk = new Chunk();

	BlockType* StoneType = new BlockType();
	StoneType->Texture[0].x = 1;

	BlockType* DirtType = new BlockType();
	DirtType->Texture[0].x = 2;

	BlockType* GrassType = new BlockType();
	GrassType->HasMultipleTextureFaces = true;
	GrassType->Texture[(int)BlockFace::Bottom].x = 2;
	GrassType->Texture[(int)BlockFace::Left].x = 3;
	GrassType->Texture[(int)BlockFace::Right].x = 3;
	GrassType->Texture[(int)BlockFace::Front].x = 3;
	GrassType->Texture[(int)BlockFace::Back].x = 3;

	BlockType* LightType = new BlockType();
	LightType->Texture[0].y = 4;

	FastNoise noise;
	noise.SetNoiseType(FastNoise::PerlinFractal);

	for (int x = 0; x < Chunk::ChunkWide; x++)
	{
		for (int z = 0; z < Chunk::ChunkDepth; z++)
		{
			float scale = 1.0f;
			float val = (noise.GetNoise(x * scale, z * scale) + 1.0f) / 2.0f;
			int maxY = round(val * (float)Chunk::ChunkTall);

			float dirtVal = (noise.GetNoise(x * 10.0f, z * 10.0f) + 1.0f) / 2.0f;

			for (int y = 0; y < maxY; y++)
			{
				if ((maxY - y) < round((1.0 - dirtVal) * 5.0f))
				{
					chunk->SetBlock(x, y, z, Block(DirtType));
				}
				else
				{
					chunk->SetBlock(x, y, z, Block(StoneType));
				}
			}

			chunk->SetBlock(x, maxY, z, Block(GrassType));

			if (x == 5 && z == 5)
			{
				chunk->SetBlock(x, maxY + 1, z, Block(LightType));
				chunk->GetBlock(x, maxY + 1, z).LightLevel = 255;

				chunk->SetBlock(x, maxY + 3, z, Block(GrassType));
			}

			if (x == 16 && z == 16)
			{
				chunk->SetBlock(x, maxY + 1, z, Block(LightType));
				chunk->GetBlock(x, maxY + 1, z).LightLevel = 255;
			}

			/*for (int y = 0; y < Chunk::ChunkTall; y++)
			{
				float val = (noise.GetNoise(x * scale, y * scale, z * scale) + 1.0f) / 2.0f;
				//val *= 1.0f - pow((float)y / (float)Chunk::ChunkTall, 1);

				int state = round(val * (float)Chunk::ChunkTall);

				if (state > y + 1) {
					chunk->SetBlock(x, y, z, Block(DirtType));
				} else if (state > y)
				{
					chunk->SetBlock(x, y, z, Block(GrassType));
				}
			}*/
		}
	}

	chunk->UpdateLighting();

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
					AddFace(mesh, *chunk, block, x, y, z, TopBlockPattern, BlockFace::Top);
				}

				if (IsAir(bottomBlock))
				{
					AddFace(mesh, *chunk, block, x, y, z, BottomBlockPattern, BlockFace::Bottom);
				}

				if (IsAir(leftBlock))
				{
					AddFace(mesh, *chunk, block, x, y, z, LeftBlockPattern, BlockFace::Left);
				}

				if (IsAir(rightBlock))
				{
					AddFace(mesh, *chunk, block, x, y, z, RightBlockPattern, BlockFace::Right);
				}

				if (IsAir(frontBlock))
				{
					AddFace(mesh, *chunk, block, x, y, z, FrontBlockPattern, BlockFace::Front);
				}

				if (IsAir(backBlock))
				{
					AddFace(mesh, *chunk, block, x, y, z, BackBlockPattern, BlockFace::Back);
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

	staticMaterial.Material->SetTexture("_TerrainTexture", Engine->GetAssetManager()->GetTexture("Assets/Textures/terrain.png"));
	//staticMaterial.Material->SetSampler("_PixelSampler", Engine->GetGraphics()->CreateSampler("_PixelSampler", WrapMode::WRAP_MODE_BORDER, WrapMode::WRAP_MODE_BORDER, WrapMode::WRAP_MODE_BORDER, false, false, true));

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
