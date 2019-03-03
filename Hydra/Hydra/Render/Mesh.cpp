#include "Mesh.h"

namespace Hydra
{
	Mesh::Mesh() : PrimitiveType(NVRHI::PrimitiveType::TRIANGLE_LIST)
	{
	}

	void Mesh::UpdateBounds()
	{
		float floatMax = FloatMax;
		float floatMin = FloatMin;

		Vector3 maxBounds = Vector3(floatMin, floatMin, floatMin);
		Vector3 minBounds = Vector3(floatMax, floatMax, floatMax);

		for (Vector3& pos : Vertices)
		{
			minBounds = glm::min(minBounds, pos);
			maxBounds = glm::max(maxBounds, pos);
		}

		Bounds = {};
		Bounds.Origin = (minBounds + maxBounds) * 0.5f;
		Bounds.Extent = maxBounds - Bounds.Origin;
	}

	void Mesh::GenerateNormals()
	{
		List<Vector3> normals;
		normals.resize(Vertices.size());

		for (int i = 0; i < Indices.size() / 3; i++)
		{
			int n0 = Indices[i * 3 + 0];
			int n1 = Indices[i * 3 + 1];
			int n2 = Indices[i * 3 + 2];

			Vector3& v0 = Vertices[n0];
			Vector3& v1 = Vertices[n1];
			Vector3& v2 = Vertices[n2];

			Vector3 normal = ComputeTriangleNormal(v0, v1, v2);

			normals[n0] = normal;
			normals[n1] = normal;
			normals[n2] = normal;
		}

		Normals = normals;
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

				mesh->Vertices = {
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

				mesh->TexCoords = {
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

				mesh->Normals = {
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
					mesh->Vertices.push_back({ vertices[i * 8 + 0], vertices[i * 8 + 1], vertices[i * 8 + 2] });
					mesh->Normals.push_back({ vertices[i * 8 + 3], vertices[i * 8 + 4], vertices[i * 8 + 5] });
					mesh->TexCoords.push_back({ vertices[i * 8 + 6], vertices[i * 8 + 7] });
					mesh->Indices.push_back(i++);
				}

				return mesh;

				break;
			}
		}

		return nullptr;
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