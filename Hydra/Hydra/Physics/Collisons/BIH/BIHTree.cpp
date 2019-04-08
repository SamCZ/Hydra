#include "Hydra/Physics/Collisons/BIH/BIHTree.h"
#include "Hydra/Physics/Collisons/BIH/BIHNode.h"

#include "Hydra/Render/Mesh.h"
#include "Hydra/Core/Math/Box.h"

namespace Hydra
{
	BIHTree::BIHTree(Mesh * mesh, int maxTrisPerNode) : _MaxTrisPerNode(maxTrisPerNode)
	{
		InitTriangles(mesh);
		ConstructRootNode();
	}

	BIHTree::BIHTree(Mesh * mesh) : _MaxTrisPerNode(MAX_TRIS_PER_NODE)
	{
		InitTriangles(mesh);
		ConstructRootNode();
	}

	BIHTree::~BIHTree()
	{
		delete _Root;
	}

	void BIHTree::GetTriangle(int index, Vector3 & v1, Vector3 & v2, Vector3 & v3)
	{
		int pointIndex = index * 9;

		v1.x = _PointData[pointIndex++];
		v1.y = _PointData[pointIndex++];
		v1.z = _PointData[pointIndex++];

		v2.x = _PointData[pointIndex++];
		v2.y = _PointData[pointIndex++];
		v2.z = _PointData[pointIndex++];

		v3.x = _PointData[pointIndex++];
		v3.y = _PointData[pointIndex++];
		v3.z = _PointData[pointIndex++];
	}

	int BIHTree::GetTriangleIndex(int triIndex)
	{
		return _TriIndices[triIndex];
	}

	int BIHTree::CollideWithRay(const Ray & r, const Matrix4& worldMatrix, Box* worldBound, CollisionResults & results)
	{
		if (worldBound)
		{
			_BBoxTempRes.Clear();

			if (worldBound->CollideWithRay(r, _BBoxTempRes))
			{
				float tMin = _BBoxTempRes.GetClosestCollision().Distance;
				float tMax = _BBoxTempRes.GetFarthestCollision().Distance;

				if (tMax <= 0)
				{
					tMax = FloatInf;
				}
				else if (tMin == tMax)
				{
					tMin = 0;
				}

				if (tMin <= 0)
				{
					tMin = 0;
				}

				if (r.Limit < FloatInf)
				{
					tMax = glm::min(tMax, r.Limit);
					if (tMin > tMax)
					{
						return 0;
					}
				}

				return this->_Root->IntersectWhere(r, worldMatrix, this, tMin, tMax, results);
			}
		}

		return 0;
	}

	Box BIHTree::CreateBox(int l, int r)
	{
		float fmax = FloatMin;
		float fmin = FloatMax;

		Vector3 max = Vector3(fmin, fmin, fmin);
		Vector3 min = Vector3(fmax, fmax, fmax);

		Vector3 v1;
		Vector3 v2;
		Vector3 v3;

		for (int i = l; i <= r; i++)
		{
			GetTriangle(i, v1, v2, v3);
			Box::CheckMinMax(min, max, v1);
			Box::CheckMinMax(min, max, v2);
			Box::CheckMinMax(min, max, v3);
		}

		return Box(BBMM min, max);
	}

	BIHNode* BIHTree::CreateNode(int l, int r, const Box & nodeBbox, int depth)
	{
		if ((r - l) < _MaxTrisPerNode || depth > MAX_TREE_DEPTH)
		{
			return new BIHNode(l, r);
		}
		Box currentBox = CreateBox(l, r);

		glm::vec3 exteriorExt = nodeBbox.GetExtent();
		glm::vec3 interiorExt = currentBox.GetExtent();

		exteriorExt -= interiorExt;

		int axis = 0;
		if (exteriorExt.x > exteriorExt.y)
		{
			if (exteriorExt.x > exteriorExt.z)
			{
				axis = 0;
			}
			else
			{
				axis = 2;
			}
		}
		else
		{
			if (exteriorExt.y > exteriorExt.z)
			{
				axis = 1;
			}
			else
			{
				axis = 2;
			}
		}
		if (exteriorExt == glm::vec3())
		{
			axis = 0;
		}

		float split = currentBox.GetOrigin()[axis];
		int pivot = SortTriangles(l, r, split, axis);
		if (pivot == l || pivot == r)
		{
			pivot = (r + l) / 2;
		}

		if (pivot < l)
		{
			//Only right
			Box rbbox = Box(currentBox);
			SetMinMax(rbbox, true, axis, split);
			return CreateNode(l, r, rbbox, depth + 1);
		}
		else if (pivot > r)
		{
			//Only left
			Box lbbox = Box(currentBox);
			SetMinMax(lbbox, false, axis, split);
			return CreateNode(l, r, lbbox, depth + 1);
		}
		else
		{
			//Build the node
			BIHNode* node = new BIHNode(axis);

			//Left child
			Box lbbox = Box(currentBox);
			SetMinMax(lbbox, false, axis, split);

			//The left node right border is the plane most right
			node->SetLeftPlane(GetMinMax(CreateBox(l, glm::max(l, pivot - 1)), false, axis));
			node->SetLeftChild(CreateNode(l, glm::max(l, pivot - 1), lbbox, depth + 1)); //Recursive call

																						 //Right Child
			Box rbbox = Box(currentBox);
			SetMinMax(rbbox, true, axis, split);
			//The right node left border is the plane most left
			node->SetRightPlane(GetMinMax(CreateBox(pivot, r), true, axis));
			node->SetRightChild(CreateNode(pivot, r, rbbox, depth + 1)); //Recursive call

			return node;
		}

		return nullptr;
	}

	void BIHTree::SetMinMax(Box & bbox, bool doMin, int axis, float value)
	{
		glm::vec3 min = bbox.GetMin();
		glm::vec3 max = bbox.GetMax();

		if (doMin)
		{
			min[axis] = value;
		}
		else
		{
			max[axis] = value;
		}

		bbox.SetMinMax(min, max);
	}

	float BIHTree::GetMinMax(Box & bbox, bool doMin, int axis)
	{
		if (doMin)
		{
			return bbox.GetMin()[axis];
		}
		else
		{
			return bbox.GetMax()[axis];
		}
	}

	int BIHTree::SortTriangles(int l, int r, float split, int axis)
	{
		int pivot = l;
		int j = r;

		glm::vec3 v1;
		glm::vec3 v2;
		glm::vec3 v3;

		while (pivot <= j)
		{
			GetTriangle(pivot, v1, v2, v3);
			v1 += v2;
			v1 += v3;
			v1 *= 1.0f / 3.0f;
			if (v1[axis] > split)
			{
				SwapTriangles(pivot, j);
				--j;
			}
			else
			{
				++pivot;
			}
		}
		pivot = (pivot == l && j < pivot) ? j : pivot;

		return pivot;
	}

	void BIHTree::SwapTriangles(int index1, int index2)
	{
		int p1 = index1 * 9;
		int p2 = index2 * 9;

		ArrayCopy(_PointData, p1, _BihSwapTmp, 0, 9);
		// copy p2 to p1
		ArrayCopy(_PointData, p2, _PointData, p1, 9);
		// copy tmp to p2
		ArrayCopy(_BihSwapTmp, 0, _PointData, p2, 9);

		int tmp2 = _TriIndices[index1];
		_TriIndices[index1] = _TriIndices[index2];
		_TriIndices[index2] = tmp2;
	}

	void BIHTree::ArrayCopy(float * src, int srcPos, float * dest, int destPos, int length)
	{
		while (length > 0)
		{
			dest[destPos] = src[srcPos];
			srcPos++;
			destPos++;
			length--;
		}
	}

	void BIHTree::InitTriangles(Mesh* mesh)
	{
		_NumTris = (int)mesh->Indices.size();
		_NumPointData = _NumTris * 3 * 3;

		// Fill vertex data
		_PointData = new float[_NumPointData];

		int p = 0;
		for (VertexBufferEntry& e : mesh->VertexData)
		{
			_PointData[p++] = e.position.x;
			_PointData[p++] = e.position.y;
			_PointData[p++] = e.position.z;
		}

		// Fill index data
		_NumTriIndices = _NumTris;

		_TriIndices = new int[_NumTriIndices];

		for (int i = 0; i < _NumTriIndices; i++)
		{
			_TriIndices[i] = (int)mesh->Indices[i];
		}
	}

	void BIHTree::ConstructRootNode()
	{
		Box sceneBbox = CreateBox(0, _NumTris - 1);
		_Root = CreateNode(0, _NumTris - 1, sceneBbox, 0);
	}
}