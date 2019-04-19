#include "Mesh.h"
#include "Hydra/EngineContext.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <list>

#include "Hydra/Core/Math/Triangle.h"

#include "Hydra/Physics/Collisons/BIH/BIHTree.h"

Mesh::Mesh() : PrimitiveType(NVRHI::PrimitiveType::TRIANGLE_LIST), _AutoCreateBuffers(true), _IndexHandle(nullptr), _VertexBuffer(nullptr), _IsIndexed(true), _ComplexCollider(nullptr)
{
}

Mesh::~Mesh()
{
	if (_ComplexCollider != nullptr)
	{
		delete _ComplexCollider;
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
		Vector3 normal = Triangle::ComputeTriangleNormal(vb0.position, vb1.position, vb2.position);

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

void Mesh::CreateComplexCollider()
{
	if (_ComplexCollider == nullptr)
	{
		UpdateBounds();

		_ComplexCollider = new BIHTree(this);
	}
}

BIHTree* Mesh::GetComplexCollider()
{
	return _ComplexCollider;
}

void Mesh::UpdateBuffers(EngineContext* context)
{
	if (_IndexHandle != nullptr)
	{
		context->GetRenderInterface()->destroyBuffer(_IndexHandle);
	}

	if (_VertexBuffer != nullptr)
	{
		context->GetRenderInterface()->destroyBuffer(_VertexBuffer);
	}

	if (VertexData.size() == 0 || Indices.size() == 0) return;

	_IndexCount = Indices.size();

	NVRHI::BufferDesc indexBufferDesc;
	indexBufferDesc.isIndexBuffer = true;
	indexBufferDesc.byteSize = uint32_t(Indices.size() * sizeof(unsigned int));
	_IndexHandle = context->GetRenderInterface()->createBuffer(indexBufferDesc, &Indices[0]);


	NVRHI::BufferDesc vertexBufferDesc;
	vertexBufferDesc.isVertexBuffer = true;
	vertexBufferDesc.byteSize = uint32_t(VertexData.size() * sizeof(VertexBufferEntry));
	_VertexBuffer = context->GetRenderInterface()->createBuffer(vertexBufferDesc, &VertexData[0]);
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

bool Mesh::CanAutoCreateBuffers() const
{
	return _AutoCreateBuffers;
}

void Mesh::SetAutoCreateBuffers(bool state)
{
	_AutoCreateBuffers = state;
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

int Mesh::GetIndexCount() const
{
	return _IndexCount;
}

Vector3 Mesh::ComputeNormalFromMultiplePoints(List<Vector3>& points)
{

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