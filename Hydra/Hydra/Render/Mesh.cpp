#include "Mesh.h"
#include "Hydra/Engine.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <list>

namespace Hydra
{
	Mesh::Mesh() : PrimitiveType(NVRHI::PrimitiveType::TRIANGLE_LIST), _IndexHandle(nullptr), _VertexBuffer(nullptr), _IsIndexed(true)
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

	void Mesh::GenerateUVs()
	{
		for (int i = 0; i < Indices.size() / 3; i++)
		{
			int n0 = Indices[i * 3 + 0];
			int n1 = Indices[i * 3 + 1];
			int n2 = Indices[i * 3 + 2];

			VertexBufferEntry& vb0 = VertexData[n0];
			VertexBufferEntry& vb1 = VertexData[n1];
			VertexBufferEntry& vb2 = VertexData[n2];

			vb0.texCoord = Vector2(0, 0);
			vb1.texCoord = Vector2(0, 1);
			vb2.texCoord = Vector2(1, 1);
		}
	}

	void Mesh::GenerateNormals()
	{
		Vector2 uv1 = Vector2(0, 0);
		Vector2 uv2 = Vector2(0, 1);
		Vector2 uv3 = Vector2(1, 1);
		
		for (int i = 0; i < Indices.size() / 3; i++)
		{
			int n0 = Indices[i * 3 + 0];
			int n1 = Indices[i * 3 + 1];
			int n2 = Indices[i * 3 + 2];

			VertexBufferEntry& vb0 = VertexData[n0];
			VertexBufferEntry& vb1 = VertexData[n1];
			VertexBufferEntry& vb2 = VertexData[n2];

			// TANGENT SPACE
			glm::vec3 deltaPos1 = vb1.position - vb0.position;
			glm::vec3 deltaPos2 = vb2.position - vb0.position;

			glm::vec2 deltaUV1 = uv2 - uv1;
			glm::vec2 deltaUV2 = uv3 - uv1;

			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
			glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

			vb0.tangent = tangent;
			vb1.tangent = tangent;
			vb2.tangent = tangent;

			vb0.bitangent = bitangent;
			vb1.bitangent = bitangent;
			vb2.bitangent = bitangent;

			// NORMAL
			Vector3 normal = ComputeTriangleNormal(vb0.position, vb1.position, vb2.position);

			vb0.normal = normal;
			vb1.normal = normal;
			vb2.normal = normal;
		}
	}

	void Mesh::GenerateTangentsAndBiTangents()
	{
		for (int i = 0; i < Indices.size() / 3; i++)
		{
			int n0 = Indices[i * 3 + 0];
			int n1 = Indices[i * 3 + 1];
			int n2 = Indices[i * 3 + 2];

			VertexBufferEntry& vb0 = VertexData[n0];
			VertexBufferEntry& vb1 = VertexData[n1];
			VertexBufferEntry& vb2 = VertexData[n2];

			// TANGENT SPACE
			glm::vec3 deltaPos1 = vb1.position - vb0.position;
			glm::vec3 deltaPos2 = vb2.position - vb0.position;

			glm::vec2 deltaUV1 = vb1.texCoord - vb0.texCoord;
			glm::vec2 deltaUV2 = vb2.texCoord - vb0.texCoord;

			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
			glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

			vb0.tangent = tangent;
			vb1.tangent = tangent;
			vb2.tangent = tangent;

			vb0.bitangent = bitangent;
			vb1.bitangent = bitangent;
			vb2.bitangent = bitangent;
		}
	}

	struct Tri
	{
		Vector3 P0;
		Vector3 P1;
		Vector3 P2;
	};

	size_t HashV3(const Vector3& vec)
	{
		return std::hash<Vector3>()(vec);
	}

	void Mesh::SmoothMesh()
	{
		std::map<size_t, List<Vector3>> VertexMap;

		for (int i = 0; i < VertexData.size() / 3; i++)
		{
			Vector3 p0 = VertexData[i * 3 + 0].position;
			Vector3 p1 = VertexData[i * 3 + 1].position;
			Vector3 p2 = VertexData[i * 3 + 2].position;

			VertexMap[HashV3(p0)].push_back(p1);
			VertexMap[HashV3(p0)].push_back(p2);

			VertexMap[HashV3(p1)].push_back(p0);
			VertexMap[HashV3(p1)].push_back(p2);

			VertexMap[HashV3(p2)].push_back(p0);
			VertexMap[HashV3(p2)].push_back(p1);
		}

		Map<size_t, Vector3> SummedVertices;
		
		ITER(VertexMap, it)
		{
			//it->second.unique();

			Vector3 pos;

			for (Vector3& v : it->second)
			{
				pos += v;
			}

			pos /= (float)it->second.size();

			SummedVertices[it->first] = pos;
		}

		/*Map<size_t, Vector3> VerticesNormals;
		ITER(VertexMap, it)
		{
			VerticesNormals[it->first] = ComputeNormalFromMultiplePoints(it->second);
		}*/

		for (int i = 0; i < VertexData.size(); i++)
		{
			Vector3 p = VertexData[i].position;
			if (p.x > 0.0f && p.z > 0.0f && p.x < 15.0f && p.z < 15.0f)
			{
				VertexData[i].position = SummedVertices[HashV3(p)];
			}

			/*VertexData[i * 3 + 0].normal = VerticesNormals[HashV3(p0)];*/
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

		_IndexCount = Indices.size();

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

	void Mesh::SetIndexBuffer(NVRHI::BufferHandle buffer)
	{
		_IndexHandle = buffer;
	}

	void Mesh::SetVertexBuffer(NVRHI::BufferHandle buffer)
	{
		_VertexBuffer = buffer;
	}

	void Mesh::SetIndexed(bool indexed)
	{
		_IsIndexed = indexed;
	}

	bool Mesh::IsIndexed()
	{
		return _IsIndexed;
	}

	void Mesh::SetIndexCount(int count)
	{
		_IndexCount = count;
	}

	int Hydra::Mesh::GetIndexCount() const
	{
		return _IndexCount;
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

	void Find_ScatterMatrix(
		const List<Vector4> &Points,
		const Vector3 &Centroid,
		float ScatterMatrix[3][3],
		int Order[3]
	)
	{
		int    i, TempI;
		float    TempD;

		/*    To compute the correct scatter matrix, the centroid must be
		**    subtracted from all points.  If the plane is to be forced to pass
		**    through the origin (0,0,0), then the Centroid was earlier set
		**    equal to (0,0,0).  This, of course, is NOT the true Centroid of
		**    the set of points!  Since the matrix is symmetrical about its
		**    diagonal, one-third of it is redundant and is simply found at
		**    the end.
		*/
		for (i = 0; i < 3; i++)
		{
			ScatterMatrix[i][0] = ScatterMatrix[i][1] = ScatterMatrix[i][2] = 0;
		}

		for (UINT i = 0; i < Points.size(); i++)
		{
			const Vector4 &P = Points[i];
			Vector3 d = Vector3(P.x, P.y, P.z) - Centroid;
			float Weight = P.w;
			ScatterMatrix[0][0] += d.x*d.x*Weight;
			ScatterMatrix[0][1] += d.x*d.y*Weight;
			ScatterMatrix[0][2] += d.x*d.z*Weight;
			ScatterMatrix[1][1] += d.y*d.y*Weight;
			ScatterMatrix[1][2] += d.y*d.z*Weight;
			ScatterMatrix[2][2] += d.z*d.z*Weight;
		}
		ScatterMatrix[1][0] = ScatterMatrix[0][1];
		ScatterMatrix[2][0] = ScatterMatrix[0][2];
		ScatterMatrix[2][1] = ScatterMatrix[1][2];

		/*    Now, perform a sort of "Matrix-sort", whereby all the larger elements
		**    in the matrix are relocated towards the lower-right portion of the
		**    matrix.  This is done as a requisite of the tred2 and tqli algorithms,
		**    for which the scatter matrix is being computed as an input.
		**    "Order" is a 3 element array that will keep track of the xyz order
		**    in the ScatterMatrix.
		*/
		Order[0] = 0;        /* Beginning order is x-y-z, as found above */
		Order[1] = 1;
		Order[2] = 2;
		if (ScatterMatrix[0][0] > ScatterMatrix[1][1])
		{
			TempD = ScatterMatrix[0][0];
			ScatterMatrix[0][0] = ScatterMatrix[1][1];
			ScatterMatrix[1][1] = TempD;
			TempD = ScatterMatrix[0][2];
			ScatterMatrix[0][2] = ScatterMatrix[2][0] = ScatterMatrix[1][2];
			ScatterMatrix[1][2] = ScatterMatrix[2][1] = TempD;
			TempI = Order[0];
			Order[0] = Order[1];
			Order[1] = TempI;
		}
		if (ScatterMatrix[1][1] > ScatterMatrix[2][2])
		{
			TempD = ScatterMatrix[1][1];
			ScatterMatrix[1][1] = ScatterMatrix[2][2];
			ScatterMatrix[2][2] = TempD;
			TempD = ScatterMatrix[0][1];
			ScatterMatrix[0][1] = ScatterMatrix[1][0] = ScatterMatrix[0][2];
			ScatterMatrix[0][2] = ScatterMatrix[2][0] = TempD;
			TempI = Order[1];
			Order[1] = Order[2];
			Order[2] = TempI;
		}
		if (ScatterMatrix[0][0] > ScatterMatrix[1][1])
		{
			TempD = ScatterMatrix[0][0];
			ScatterMatrix[0][0] = ScatterMatrix[1][1];
			ScatterMatrix[1][1] = TempD;
			TempD = ScatterMatrix[0][2];
			ScatterMatrix[0][2] = ScatterMatrix[2][0] = ScatterMatrix[1][2];
			ScatterMatrix[1][2] = ScatterMatrix[2][1] = TempD;
			TempI = Order[0];
			Order[0] = Order[1];
			Order[1] = TempI;
		}
	}

#define    SIGN(a,b)    ((b)<0? -fabs(a):fabs(a))

	void tred2(float a[3][3], float d[3], float e[3])
	{
		int        l, k, i, j;
		float    scale, hh, h, g, f;

		for (i = 3; i >= 2; i--)
		{
			l = i - 1;
			h = scale = 0.0;
			if (l > 1)
			{
				for (k = 1; k <= l; k++)
					scale += fabs(a[i - 1][k - 1]);
				if (scale == 0.0)        /* skip transformation */
					e[i - 1] = a[i - 1][l - 1];
				else
				{
					for (k = 1; k <= l; k++)
					{
						a[i - 1][k - 1] /= scale;    /* use scaled a's for transformation. */
						h += a[i - 1][k - 1] * a[i - 1][k - 1];    /* form sigma in h. */
					}
					f = a[i - 1][l - 1];
					g = f > 0 ? -sqrt(h) : sqrt(h);
					e[i - 1] = scale * g;
					h -= f * g;    /* now h is equation (11.2.4) */
					a[i - 1][l - 1] = f - g;    /* store u in the ith row of a. */
					f = 0.0;
					for (j = 1; j <= l; j++)
					{
						a[j - 1][i - 1] = a[i - 1][j - 1] / h; /* store u/H in ith column of a. */
						g = 0.0;    /* form an element of A.u in g */
						for (k = 1; k <= j; k++)
							g += a[j - 1][k - 1] * a[i - 1][k - 1];
						for (k = j + 1; k <= l; k++)
							g += a[k - 1][j - 1] * a[i - 1][k - 1];
						e[j - 1] = g / h; /* form element of p in temorarliy unused element of e. */
						f += e[j - 1] * a[i - 1][j - 1];
					}
					hh = f / (h + h);    /* form K, equation (11.2.11) */
					for (j = 1; j <= l; j++) /* form q and store in e overwriting p. */
					{
						f = a[i - 1][j - 1]; /* Note that e[l]=e[i-1] survives */
						e[j - 1] = g = e[j - 1] - hh * f;
						for (k = 1; k <= j; k++) /* reduce a, equation (11.2.13) */
							a[j - 1][k - 1] -= (f*e[k - 1] + g * a[i - 1][k - 1]);
					}
				}
			}
			else
				e[i - 1] = a[i - 1][l - 1];
			d[i - 1] = h;
		}


		/*
		**    For computing eigenvector.
		*/
		d[0] = 0.0;
		e[0] = 0.0;

		for (i = 1; i <= 3; i++)/* begin accumualting of transfomation matrices */
		{
			l = i - 1;
			if (d[i - 1]) /* this block skipped when i=1 */
			{
				for (j = 1; j <= l; j++)
				{
					g = 0.0;
					for (k = 1; k <= l; k++) /* use u and u/H stored in a to form P.Q */
						g += a[i - 1][k - 1] * a[k - 1][j - 1];
					for (k = 1; k <= l; k++)
						a[k - 1][j - 1] -= g * a[k - 1][i - 1];
				}
			}
			d[i - 1] = a[i - 1][i - 1];
			a[i - 1][i - 1] = 1.0; /* reset row and column of a to identity matrix for next iteration */
			for (j = 1; j <= l; j++)
				a[j - 1][i - 1] = a[i - 1][j - 1] = 0.0;
		}
	}

	void tqli(float d[3], float e[3], float z[3][3])
	{
		int        m, l, iter, i, k;
		float    s, r, p, g, f, dd, c, b;

		for (i = 2; i <= 3; i++)
			e[i - 2] = e[i - 1];    /* convenient to renumber the elements of e */
		e[2] = 0.0;
		for (l = 1; l <= 3; l++)
		{
			iter = 0;
			do
			{
				for (m = l; m <= 2; m++)
				{
					/*
					**    Look for a single small subdiagonal element
					**    to split the matrix.
					*/
					dd = fabs(d[m - 1]) + fabs(d[m]);
					if (fabs(e[m - 1]) + dd == dd)
						break;
				}
				if (m != l)
				{
					if (iter++ == 30)
					{
						printf("\nToo many iterations in TQLI");
					}
					g = (d[l] - d[l - 1]) / (2.0f*e[l - 1]); /* form shift */
					r = sqrt((g*g) + 1.0f);
					g = d[m - 1] - d[l - 1] + e[l - 1] / (g + SIGN(r, g)); /* this is dm-ks */
					s = c = 1.0;
					p = 0.0;
					for (i = m - 1; i >= l; i--)
					{
						/*
						**    A plane rotation as in the original
						**    QL, followed by Givens rotations to
						**    restore tridiagonal form.
						*/
						f = s * e[i - 1];
						b = c * e[i - 1];
						if (fabs(f) >= fabs(g))
						{
							c = g / f;
							r = sqrt((c*c) + 1.0f);
							e[i] = f * r;
							c *= (s = 1.0f / r);
						}
						else
						{
							s = f / g;
							r = sqrt((s*s) + 1.0f);
							e[i] = g * r;
							s *= (c = 1.0f / r);
						}
						g = d[i] - p;
						r = (d[i - 1] - g)*s + 2.0f*c*b;
						p = s * r;
						d[i] = g + p;
						g = c * r - b;
						for (k = 1; k <= 3; k++)
						{
							/*
							**    Form eigenvectors
							*/
							f = z[k - 1][i];
							z[k - 1][i] = s * z[k - 1][i - 1] + c * f;
							z[k - 1][i - 1] = c * z[k - 1][i - 1] - s * f;
						}
					}
					d[l - 1] = d[l - 1] - p;
					e[l - 1] = g;
					e[m - 1] = 0.0f;
				}
			} while (m != l);
		}
	}

	float UnsignedDistance(const Vector3 &a, const Vector3 &Pt)
	{
		return glm::abs(a.x * Pt.x + a.y * Pt.y + a.z * Pt.z);
	}

	struct Plane
	{
		Vector3 Origin;
		Vector3 Normal;
	};

	Plane FitToPoints(const List<Vector4> &Points, Vector3 &Basis1, Vector3 &Basis2, float &NormalEigenvalue, float &ResidualError)
	{
		Vector3 Centroid, Normal;

		float ScatterMatrix[3][3];
		int  Order[3];
		float DiagonalMatrix[3];
		float OffDiagonalMatrix[3];

		// Find centroid
		float TotalWeight = 0.0f;
		for (UINT i = 0; i < Points.size(); i++)
		{
			TotalWeight += Points[i].w;
			Centroid += Vector3(Points[i].x, Points[i].y, Points[i].z) * Points[i].w;
		}
		Centroid /= TotalWeight;

		// Compute scatter matrix
		Find_ScatterMatrix(Points, Centroid, ScatterMatrix, Order);

		tred2(ScatterMatrix, DiagonalMatrix, OffDiagonalMatrix);
		tqli(DiagonalMatrix, OffDiagonalMatrix, ScatterMatrix);

		/*
		**    Find the smallest eigenvalue first.
		*/
		float Min = DiagonalMatrix[0];
		float Max = DiagonalMatrix[0];
		UINT MinIndex = 0;
		UINT MiddleIndex = 0;
		UINT MaxIndex = 0;
		for (UINT i = 1; i < 3; i++)
		{
			if (DiagonalMatrix[i] < Min)
			{
				Min = DiagonalMatrix[i];
				MinIndex = i;
			}
			if (DiagonalMatrix[i] > Max)
			{
				Max = DiagonalMatrix[i];
				MaxIndex = i;
			}
		}
		for (UINT i = 0; i < 3; i++)
		{
			if (MinIndex != i && MaxIndex != i)
			{
				MiddleIndex = i;
			}
		}
		/*
		**    The normal of the plane is the smallest eigenvector.
		*/
		for (UINT i = 0; i < 3; i++)
		{
			Normal[Order[i]] = ScatterMatrix[i][MinIndex];
			Basis1[Order[i]] = ScatterMatrix[i][MiddleIndex];
			Basis2[Order[i]] = ScatterMatrix[i][MaxIndex];
		}
		NormalEigenvalue = glm::abs(DiagonalMatrix[MinIndex]);
		Basis1 *= DiagonalMatrix[MiddleIndex];
		Basis2 *= DiagonalMatrix[MaxIndex];

		

		/*if (!Basis1.Valid() || !Basis2.Valid() || !Normal.Valid())
		{
			*this = ConstructFromPointNormal(Centroid, Vec3f::eX);
			Basis1 = Vector3(0, 1, 0);
			Basis2 = Vector3(0, 0, 1);
		}
		else
		{
			*this = ConstructFromPointNormal(Centroid, Normal);
		}*/

		Plane plane = { Centroid, Normal };

		ResidualError = 0.0f;
		for (UINT i = 0; i < Points.size(); i++)
		{
			ResidualError += UnsignedDistance(Centroid, Vector3(Points[i].x, Points[i].y, Points[i].z));
		}
		ResidualError /= Points.size();

		return plane;
	}

	Plane FitToPoints(List<Vector3> &Points, float &ResidualError)
	{
		Vector3 Basis1, Basis2;
		List<Vector4> WeightedPoints(Points.size());
		for (UINT i = 0; i < Points.size(); i++)
		{
			WeightedPoints[i] = Vector4(Points[i], 1.0f);
		}
		float NormalEigenvalue;
		return FitToPoints(WeightedPoints, Basis1, Basis2, NormalEigenvalue, ResidualError);
	}

	Vector3 Hydra::Mesh::ComputeNormalFromMultiplePoints(List<Vector3>& points)
	{
		/*if (true)
		{
			float error;
			Plane p = FitToPoints(points, error);

			return p.Origin;
		}*/

		int n = (int)points.size();

		if (n < 3)
		{
			return Vector3(0, 0, 0);
		}

		Vector3 sum = Vector3(0, 0, 0);

		for (Vector3& p : points)
		{
			sum += p;
		}

		Vector3 centroid = sum * (1.0f / n);

		float xx = 0.0f;
		float xy = 0.0f;
		float xz = 0.0f;
		float yy = 0.0f;
		float yz = 0.0f;
		float zz = 0.0f;

		for (Vector3& p : points)
		{
			Vector3 r = p - centroid;
			xx += r.x * r.x;
			xy += r.x * r.y;
			xz += r.x * r.z;
			yy += r.y * r.y;
			yz += r.y * r.z;
			zz += r.z * r.z;
		}

		xx /= n;
		xy /= n;
		xz /= n;
		yy /= n;
		yz /= n;
		zz /= n;

		Vector3 weighted_dir = Vector3(0, 0, 0);

		{
			float det_x = yy * zz - yz * yz;
			Vector3 axis_dir = {
				det_x,
				xz * yz - xy * zz,
				xy * yz - xz * yy,
			};

			float weight = det_x * det_x;

			if (glm::dot(weighted_dir, axis_dir) < 0.0)
			{ 
				weight = -weight; 
			}
			weighted_dir += axis_dir * weight;
		}

		{
			float det_y = xx * zz - xz * xz;
			Vector3 axis_dir = {
				xz*yz - xy * zz,
				det_y,
				xy * xz - yz * xx,
			};
			float weight = det_y * det_y;
			if (glm::dot(weighted_dir, axis_dir) < 0.0)
			{
				weight = -weight; 
			}
			weighted_dir += axis_dir * weight;
		}

		{
			float det_z = xx * yy - xy * xy;
			Vector3 axis_dir = {
				xy*yz - xz * yy,
				xy * xz - yz * xx,
				det_z,
			};
			float weight = det_z * det_z;
			if (glm::dot(weighted_dir, axis_dir) < 0.0)
			{ 
				weight = -weight; 
			}
			weighted_dir += axis_dir * weight;
		}

		return glm::normalize(weighted_dir);
	}

}