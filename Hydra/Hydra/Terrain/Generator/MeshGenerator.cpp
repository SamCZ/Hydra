#include "Hydra/Terrain/Generator/MeshGenerator.h"

namespace Hydra
{
	int GetHeightMapIndex(int x, int y, int w)
	{
		return x + y * w;
	}

	MeshData* MeshGenerator::GenerateTerrainMesh(HeightMap* heightMapDef, const MeshSettings& meshSettings, int levelOfDetail)
	{
		int heightWidth = heightMapDef->Width;
		float* heightMap = heightMapDef->Data;

		int numVertsPerLine = meshSettings.GetNumVertsPerLine();
		float meshWorldSize = meshSettings.GetMeshWorldSize();

		int skipIncrement = (levelOfDetail == 0) ? 1 : levelOfDetail * 2;

		Vector2 topLeft = Vector2(-1.0f, 1.0f) * meshWorldSize / 2.0f;

		MeshData* meshData = new MeshData(numVertsPerLine, skipIncrement, meshSettings.UseFlatShading);

		int* vertexIndicesMap = new int[numVertsPerLine * numVertsPerLine];
		int meshVertexIndex = 0;
		int outOfMeshVertexIndex = -1;

		for (int y = 0; y < numVertsPerLine; y++)
		{
			for (int x = 0; x < numVertsPerLine; x++)
			{
				bool isOutOfMeshVertex = y == 0 || y == numVertsPerLine - 1 || x == 0 || x == numVertsPerLine - 1;
				bool isSkippedVertex = x > 2 && x < numVertsPerLine - 3 && y > 2 && y < numVertsPerLine - 3 && ((x - 2) % skipIncrement != 0 || (y - 2) % skipIncrement != 0);
				if (isOutOfMeshVertex)
				{
					vertexIndicesMap[x + y * numVertsPerLine] = outOfMeshVertexIndex;
					outOfMeshVertexIndex--;
				}
				else if (!isSkippedVertex)
				{
					vertexIndicesMap[x + y * numVertsPerLine] = meshVertexIndex;
					meshVertexIndex++;
				}
			}
		}

		for (int y = 0; y < numVertsPerLine; y++)
		{
			for (int x = 0; x < numVertsPerLine; x++)
			{
				bool isSkippedVertex = x > 2 && x < numVertsPerLine - 3 && y > 2 && y < numVertsPerLine - 3 && ((x - 2) % skipIncrement != 0 || (y - 2) % skipIncrement != 0);

				if (!isSkippedVertex)
				{
					bool isOutOfMeshVertex = y == 0 || y == numVertsPerLine - 1 || x == 0 || x == numVertsPerLine - 1;
					bool isMeshEdgeVertex = (y == 1 || y == numVertsPerLine - 2 || x == 1 || x == numVertsPerLine - 2) && !isOutOfMeshVertex;
					bool isMainVertex = (x - 2) % skipIncrement == 0 && (y - 2) % skipIncrement == 0 && !isOutOfMeshVertex && !isMeshEdgeVertex;
					bool isEdgeConnectionVertex = (y == 2 || y == numVertsPerLine - 3 || x == 2 || x == numVertsPerLine - 3) && !isOutOfMeshVertex && !isMeshEdgeVertex && !isMainVertex;

					int vertexIndex = vertexIndicesMap[x + y * numVertsPerLine];
					Vector2 percent = Vector2(x - 1.0f, y - 1.0f) / (float)(numVertsPerLine - 3);
					Vector2 vertexPosition2D = topLeft + Vector2(percent.x, -percent.y) * meshWorldSize;
					float height = heightMap[x + y * heightWidth];

					Vector2 heightMapTexCoord = Vector2(x, y);

					if (isEdgeConnectionVertex)
					{
						bool isVertical = x == 2 || x == numVertsPerLine - 3;
						int dstToMainVertexA = ((isVertical) ? y - 2 : x - 2) % skipIncrement;
						int dstToMainVertexB = skipIncrement - dstToMainVertexA;
						float dstPercentFromAToB = dstToMainVertexA / (float)skipIncrement;

						float heightMainVertexA = heightMap[GetHeightMapIndex((isVertical) ? x : x - dstToMainVertexA, (isVertical) ? y - dstToMainVertexA : y, heightWidth)];
						float heightMainVertexB = heightMap[GetHeightMapIndex((isVertical) ? x : x + dstToMainVertexB, (isVertical) ? y + dstToMainVertexB : y, heightWidth)];

						height = heightMainVertexA * (1 - dstPercentFromAToB) + heightMainVertexB * dstPercentFromAToB;
					}

					if (meshSettings.UseGPUTexturing)
					{
						//height = 0;
					}

					meshData->AddVertex(Vector3(vertexPosition2D.x, height, vertexPosition2D.y), percent, heightMapTexCoord, vertexIndex);

					bool createTriangle = x < numVertsPerLine - 1 && y < numVertsPerLine - 1 && (!isEdgeConnectionVertex || (x != 2 && y != 2));

					if (createTriangle)
					{
						int currentIncrement = (isMainVertex && x != numVertsPerLine - 3 && y != numVertsPerLine - 3) ? skipIncrement : 1;

						int a = vertexIndicesMap[x + (y * numVertsPerLine)];
						int b = vertexIndicesMap[x + currentIncrement + (y * numVertsPerLine)];
						int c = vertexIndicesMap[x + ((y + currentIncrement) * numVertsPerLine)];
						int d = vertexIndicesMap[x + currentIncrement + ((y + currentIncrement) * numVertsPerLine)];
						/*meshData->AddTriangle(a, d, c);
						meshData->AddTriangle(d, a, b);*/

						meshData->AddTriangle(c, d, a);
						meshData->AddTriangle(b, a, d);
					}
				}
			}
		}

		delete[] vertexIndicesMap;

		meshData->ProcessMesh();

		return meshData;
	}
}