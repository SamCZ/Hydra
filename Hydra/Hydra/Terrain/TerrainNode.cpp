#include "Hydra/Terrain/TerrainNode.h"
#include "Hydra/Scene/Components/Renderer.h"
#include "Hydra/Render/Mesh.h"
#include "Hydra/Scene/Components/Camera.h"

namespace Hydra
{
	static int maxlod = -1;

	TerrainNode::TerrainNode(SharedPtr<Material> material, const Vector2i& position, float size, int lod)
		: Spatial("TerrainNode(" + ToString(position.x) + ", " + ToString(position.y) + ")"), _Location(position), _Size(size), _Lod(lod)
	{
		int halfSize = size / 2;

		if (halfSize >= 1)
		{
			int nextLod = lod + 1;

			AddChild(MakeShared<TerrainNode>(material, position + Vector2i(0, 0), halfSize, nextLod));
			AddChild(MakeShared<TerrainNode>(material, position + Vector2i(0, halfSize), halfSize, nextLod));
			AddChild(MakeShared<TerrainNode>(material, position + Vector2i(halfSize, halfSize), halfSize, nextLod));
			AddChild(MakeShared<TerrainNode>(material, position + Vector2i(halfSize, 0), halfSize, nextLod));
		}

		if (lod > maxlod)
		{
			maxlod = lod;

			std::cout << maxlod << std::endl;
		}

		_Renderer = AddComponent<Renderer>();
		_Renderer->SetMesh(CreatePatch(position, size));
		//_Renderer->Material = material;


		_LodRanges[0] = 1750;
		_LodRanges[1] = 874;
		_LodRanges[2] = 386;
		_LodRanges[3] = 192;
		_LodRanges[4] = 100;
		_LodRanges[5] = 50;
		_LodRanges[6] = 0;
		_LodRanges[7] = 0;
	}

	void TerrainNode::Start()
	{
		Spatial::Start();
	}

	void TerrainNode::Update()
	{
		float halfSize = _Size * 0.5f;
		Vector3 center = Vector3(_Location.x + halfSize, 0, _Location.y + halfSize);

		float distance = glm::abs(glm::distance(center, Camera::MainCamera->GameObject->Position));

		switch (_Lod)
		{
		case 0: if (distance < _LodRanges[0])
		{
			SetIsLeaf(false);
		}
				else if (distance >= _LodRanges[0])
		{
			SetIsLeaf(true);
		}
				break;
		case 1: if (distance < _LodRanges[1])
		{
			SetIsLeaf(false);
		}
				else if (distance >= _LodRanges[1])
		{
			SetIsLeaf(true);
		}
				break;
		case 2: if (distance < _LodRanges[2])
		{
			SetIsLeaf(false);
		}
				else if (distance >= _LodRanges[2])
		{
			SetIsLeaf(true);
		}
				break;
		case 3: if (distance < _LodRanges[3])
		{
			SetIsLeaf(false);
		}
				else if (distance >= _LodRanges[3])
		{
			SetIsLeaf(true);
		}
				break;
		case 4: if (distance < _LodRanges[4])
		{
			SetIsLeaf(false);
		}
				else if (distance >= _LodRanges[4])
		{
			SetIsLeaf(true);
		}
				break;
		case 5: if (distance < _LodRanges[5])
		{
			SetIsLeaf(false);
		}
				else if (distance >= _LodRanges[5])
		{
			SetIsLeaf(true);
		}
				break;
		case 6: if (distance < _LodRanges[6])
		{
			SetIsLeaf(false);
		}
				else if (distance >= _LodRanges[6])
		{
			SetIsLeaf(true);
		}
				break;
		case 7: if (distance < _LodRanges[7])
		{
			SetIsLeaf(false);
		}
				else if (distance >= _LodRanges[7])
		{
			SetIsLeaf(true);
		}
				break;
		}

		Spatial::Update();
	}

	void TerrainNode::SetChildsEnabled(bool enabled)
	{
		for (SpatialPtr child : _Childs)
		{
			child->SetEnabled(enabled);
		}
	}

	void TerrainNode::SetIsLeaf(bool leaf)
	{
		_Renderer->Enabled = leaf;

		SetChildsEnabled(leaf == false);
	}

	Mesh* TerrainNode::CreatePatch(const Vector2i & pos, float size)
	{
		Mesh* mesh = new Mesh();

		mesh->VertexData = {
			{ Vector3(pos.x, 0, pos.y              ), Vector2(0, 0), Vector3(0, 1, 0), Vector3(0, 0, 0), Vector3(0, 0, 0) },
			{ Vector3(pos.x, 0, pos.y + size       ), Vector2(0, 0), Vector3(0, 1, 0), Vector3(0, 0, 0), Vector3(0, 0, 0) },
			{ Vector3(pos.x + size, 0, pos.y + size), Vector2(0, 0), Vector3(0, 1, 0), Vector3(0, 0, 0), Vector3(0, 0, 0) },
			{ Vector3(pos.x + size, 0, pos.y       ), Vector2(0, 0), Vector3(0, 1, 0), Vector3(0, 0, 0), Vector3(0, 0, 0) }
		};

		mesh->Indices = {
			0, 1, 2,
			0, 2, 3
		};

		//mesh->PrimitiveType = NVRHI::PrimitiveType::PATCH_3_CONTROL_POINT;

		mesh->UpdateBuffers();
		return mesh;
	}
}