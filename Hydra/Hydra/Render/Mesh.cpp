#include "Mesh.h"
#include "Hydra/Engine.h"

namespace Hydra
{
	Mesh::Mesh() : PrimitiveType(NVRHI::PrimitiveType::TRIANGLE_LIST), _IndexHandle(nullptr), _VertexBuffer(nullptr)
	{
	}

	Mesh::~Mesh()
	{
		if (_IndexHandle != nullptr)
		{
			Engine::GetRenderInterface()->destroyBuffer(_IndexHandle);
		}

		if (_VertexBuffer != nullptr)
		{
			Engine::GetRenderInterface()->destroyBuffer(_VertexBuffer);
		}
	}

	void Mesh::UpdateBounds()
	{
		float floatMax = FloatMax;
		float floatMin = FloatMin;

		Vector3 maxBounds = Vector3(floatMin, floatMin, floatMin);
		Vector3 minBounds = Vector3(floatMax, floatMax, floatMax);

		for (VertexBufferEntry& vb : VertexData)
		{
			minBounds = glm::min(minBounds, vb.position);
			maxBounds = glm::max(maxBounds, vb.position);
		}

		Bounds = {};
		Bounds.Origin = (minBounds + maxBounds) * 0.5f;
		Bounds.Extent = maxBounds - Bounds.Origin;
	}

	void Mesh::GenerateNormals()
	{
		
		for (int i = 0; i < Indices.size() / 3; i++)
		{
			int n0 = Indices[i * 3 + 0];
			int n1 = Indices[i * 3 + 1];
			int n2 = Indices[i * 3 + 2];

			VertexBufferEntry& vb0 = VertexData[n0];
			VertexBufferEntry& vb1 = VertexData[n1];
			VertexBufferEntry& vb2 = VertexData[n2];

			Vector3 normal = ComputeTriangleNormal(vb0.position, vb1.position, vb2.position);

			vb0.normal = normal;
			vb1.normal = normal;
			vb2.normal = normal;
		}
	}

	void Mesh::UpdateBuffers()
	{
		if (_IndexHandle != nullptr)
		{
			Engine::GetRenderInterface()->destroyBuffer(_IndexHandle);
		}

		if (_VertexBuffer != nullptr)
		{
			Engine::GetRenderInterface()->destroyBuffer(_VertexBuffer);
		}

		if (VertexData.size() == 0 || Indices.size() == 0) return;

		NVRHI::BufferDesc indexBufferDesc;
		indexBufferDesc.isIndexBuffer = true;
		indexBufferDesc.byteSize = uint32_t(Indices.size() * sizeof(unsigned int));
		_IndexHandle = Engine::GetRenderInterface()->createBuffer(indexBufferDesc, &Indices[0]);


		NVRHI::BufferDesc vertexBufferDesc;
		vertexBufferDesc.isVertexBuffer = true;
		vertexBufferDesc.byteSize = uint32_t(VertexData.size() * sizeof(VertexBufferEntry));
		_VertexBuffer = Engine::GetRenderInterface()->createBuffer(vertexBufferDesc, &VertexData[0]);
	}

	Mesh* Mesh::CreatePrimitive(const PrimitiveType::Enum & type, const Vector3& scale)
	{
		float xSize = 0.5f * scale.x;
		float ySize = 0.5f * scale.y;
		float zSize = 0.5f * scale.z;


		switch (type)
		{
			case PrimitiveType::Box:
			{
				Mesh* mesh = new Mesh();

				mesh->Indices = {
					0,1,2,
					0,2,3,
					4,5,6,
					4,6,7,
					8,9,10,
					8,10,11,
					12,13,14,
					12,14,15,
					16,17,18,
					16,18,19,
					20,21,22,
					20,22,23
				};

				List<Vector3> verts = {
					{ xSize,-ySize,-zSize },
					{ xSize,-ySize,zSize },
					{ -xSize,-ySize,zSize },
					{ -xSize,-ySize,-zSize },
					{ xSize,ySize,-zSize },
					{ -xSize,ySize,-zSize },
					{ -xSize,ySize,zSize },
					{ xSize,ySize,zSize },
					{ xSize,-ySize,-zSize },
					{ xSize,ySize,-zSize },
					{ xSize,ySize,zSize },
					{ xSize,-ySize,zSize },
					{ xSize,-ySize,zSize },
					{ xSize,ySize,zSize },
					{ -xSize,ySize,zSize },
					{ -xSize,-ySize,zSize },
					{ -xSize,-ySize,zSize },
					{ -xSize,ySize,zSize },
					{ -xSize,ySize,-zSize },
					{ -xSize,-ySize,-zSize },
					{ xSize,ySize,-zSize },
					{ xSize,-ySize,-zSize },
					{ -xSize,-ySize,-zSize },
					{ -xSize,ySize,-zSize }
				};

				List<Vector2> uvs = {
						{ 0,1 },
						{ 1,1 },
						{ 1,0 },
						{ 0,0 },
						{ 0,1 },
						{ 1,1 },
						{ 1,0 },
						{ 0,0 },
						{ 0,1 },
						{ 1,1 },
						{ 1,0 },
						{ 0,0 },
						{ 0,1 },
						{ 1,1 },
						{ 1,0 },
						{ 0,0 },
						{ 0,1 },
						{ 1,1 },
						{ 1,0 },
						{ 0,0 },
						{ 0,1 },
						{ 1,1 },
						{ 1,0 },
						{ 0,0 }
				};

				List<Vector3> normals = {
						{ 0,-1,0 },
						{ 0,-1,0 },
						{ 0,-1,0 },
						{ 0,-1,0 },
						{ 0,1,0 },
						{ 0,1,0 },
						{ 0,1,0 },
						{ 0,1,0 },
						{ 1,0,0 },
						{ 1,0,0 },
						{ 1,0,0 },
						{ 1,0,0 },
						{ -0,-0,1 },
						{ -0,-0,1 },
						{ -0,-0,1 },
						{ -0,-0,1 },
						{ -1,-0,-0 },
						{ -1,-0,-0 },
						{ -1,-0,-0 },
						{ -1,-0,-0 },
						{ 0,0,-1 },
						{ 0,0,-1 },
						{ 0,0,-1 },
						{ 0,0,-1 }
				};

				for (int i = 0; i < verts.size(); i++)
				{
					VertexBufferEntry entry = {};
					entry.position = verts[i];
					entry.texCoord = uvs[i];
					entry.normal = normals[i];
					mesh->VertexData.emplace_back(entry);
				}

				mesh->UpdateBuffers();

				return mesh;

				break;
			}
			case PrimitiveType::BoxNDC:
			{
				float vertices[] = {
					// back face
					-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
					 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
					 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
					 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
					-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
					-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
					// front face
					-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
					 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
					 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
					 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
					-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
					-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
					// left face
					-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
					-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
					-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
					-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
					-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
					-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
					// right face
					 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
					 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
					 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
					 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
					 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
					 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
					// bottom face
					-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
					 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
					 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
					 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
					-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
					-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
					// top face
					-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
					 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
					 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
					 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
					-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
					-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
				};

				Mesh* mesh = new Mesh();

				int index = 0;
				for (int i = 0; i < (36 * 8) / 8; i++)
				{
					VertexBufferEntry entry = {};
					entry.position = { vertices[i * 8 + 0], vertices[i * 8 + 1], vertices[i * 8 + 2] };
					entry.normal = { vertices[i * 8 + 3], vertices[i * 8 + 4], vertices[i * 8 + 5] };
					entry.texCoord = { vertices[i * 8 + 6], vertices[i * 8 + 7] };

					mesh->Indices.push_back(i++);

					mesh->VertexData.emplace_back(entry);
				}

				mesh->UpdateBuffers();

				return mesh;

				break;
			}
		}

		return nullptr;
	}

	NVRHI::BufferHandle Mesh::GetVertexBuffer()
	{
		return _VertexBuffer;
	}

	NVRHI::BufferHandle Mesh::GetIndexBuffer()
	{
		return _IndexHandle;
	}

	Vector3 Mesh::ComputeTriangleNormal(const Vector3 & p1, const Vector3 & p2, const Vector3 & p3)
	{
		Vector3 U = p2 - p1;
		Vector3 V = p3 - p1;
		float x = (U.y * V.z) - (U.z * V.y);
		float y = (U.z * V.x) - (U.x * V.z);
		float z = (U.x * V.y) - (U.y * V.x);
		return glm::normalize(Vector3(x, y, z));
	}

}