#include "Hydra/Terrain/Generator/MeshBuilder.h"
#include "Hydra/Scene/Spatial.h"
#include "Hydra/Scene/Components/Renderer.h"
#include "Hydra/Render/Mesh.h"

#include "Hydra/Render/Material.h"

namespace Hydra
{
	void MeshBuilder::AddQuad(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector2& uv0, const Vector2& uv1, const Vector2& uv2, const Vector2& uv3)
	{
		/*_VertexData.push_back({ v0, uv0, Vector3(), Vector3(), Vector3() });
		_VertexData.push_back({ v1, uv1, Vector3(), Vector3(), Vector3() });
		_VertexData.push_back({ v2, uv2, Vector3(), Vector3(), Vector3() });

		_VertexData.push_back({ v0, uv0, Vector3(), Vector3(), Vector3() });
		_VertexData.push_back({ v2, uv2, Vector3(), Vector3(), Vector3() });
		_VertexData.push_back({ v3, uv3, Vector3(), Vector3(), Vector3() });

		_Indices.push_back(squareCount++);
		_Indices.push_back(squareCount++);
		_Indices.push_back(squareCount++);

		_Indices.push_back(squareCount++);
		_Indices.push_back(squareCount++);
		_Indices.push_back(squareCount++);*/

		_VertexData.push_back({ v0, uv0, Vector3(), Vector3(), Vector3() });
		_VertexData.push_back({ v1, uv1, Vector3(), Vector3(), Vector3() });
		_VertexData.push_back({ v2, uv2, Vector3(), Vector3(), Vector3() });
		_VertexData.push_back({ v3, uv3, Vector3(), Vector3(), Vector3() });

		_Indices.push_back(squareCount * 4);
		_Indices.push_back((squareCount * 4) + 1);
		_Indices.push_back((squareCount * 4) + 3);
		_Indices.push_back((squareCount * 4) + 1);
		_Indices.push_back((squareCount * 4) + 2);
		_Indices.push_back((squareCount * 4) + 3);
		squareCount++;
	}

	void MeshBuilder::Apply(SharedPtr<Spatial> obj)
	{
		RendererPtr renderer = obj->GetComponent<Renderer>();

		if (renderer == nullptr)
		{
			renderer = obj->AddComponent<Renderer>();
		}

		Mesh* mesh = renderer->GetMesh();

		if (mesh == nullptr)
		{
			mesh = new Mesh();
			renderer->SetMesh(mesh);

			renderer->Material = Material::CreateOrGet("Assets/Shaders/VoxelTerrain.hlsl");

			renderer->TestColor = Vector3(1.0f);
		}

		mesh->VertexData = _VertexData;
		mesh->Indices = _Indices;


		mesh->GenerateNormals();
		mesh->UpdateBuffers();

		_VertexData.clear();
		_Indices.clear();

		squareCount = 0;
	}
}