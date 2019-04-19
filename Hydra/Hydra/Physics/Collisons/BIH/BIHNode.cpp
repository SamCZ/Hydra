#include "Hydra/Physics/Collisons/BIH/BIHNode.h"
#include "Hydra/Physics/Collisons/BIH/BIHTree.h"

#include "Hydra/Core/Math/Triangle.h"

struct BIHStackData
{
	BIHNode* Node;
	float Min;
	float Max;

	inline BIHStackData(BIHNode* node, float min, float max) : Node(node), Min(min), Max(max)
	{

	}
};


BIHNode::BIHNode(int l, int r) : LeftIndex(l), RightIndex(r), Axis(3), Left(nullptr), Right(nullptr)
{
}
BIHNode::BIHNode(int axis) : Axis(axis), Left(nullptr), Right(nullptr)
{
}
BIHNode::~BIHNode()
{
	delete Left;
	delete Right;
}

BIHNode * BIHNode::GetLeftChild()
{
	return Left;
}

void BIHNode::SetLeftChild(BIHNode* left)
{
	if (Left != nullptr)
	{
		delete Left;
	}

	Left = left;
}

float BIHNode::GetLeftPlane()
{
	return LeftPlane;
}

void BIHNode::SetLeftPlane(float leftPlane)
{
	LeftPlane = leftPlane;
}

BIHNode * BIHNode::GetRightChild()
{
	return Right;
}

void BIHNode::SetRightChild(BIHNode* right)
{
	if (Right != nullptr)
	{
		delete Right;
	}

	Right = right;
}

float BIHNode::GetRightPlane()
{
	return RightPlane;
}

void BIHNode::SetRightPlane(float rightPlane)
{
	RightPlane = rightPlane;
}

int BIHNode::IntersectWhere(const Ray & rr, const Matrix4 & worldMatrix, BIHTree* tree, float sceneMin, float sceneMax, CollisionResults & results)
{
	List<BIHStackData> stack;

	Ray ray(rr);

	Vector3 o = ray.Origin;
	Vector3 d = ray.Direction;

	Matrix4 inv = glm::inverse(worldMatrix);
	//Quaternion rotation = glm::toQuat(inv);

	Vector4 invOrigin = inv * Vector4(ray.Origin, 1.0f);
	Vector4 invDir = inv * Vector4(ray.Direction, 1.0f);

	ray.Origin = Vector3(invOrigin.x, invOrigin.y, invOrigin.z) / invOrigin.w;
	ray.Direction = Vector3(invDir.x, invDir.y, invDir.z) / invDir.w;

	ray.Direction = glm::normalize(ray.Direction);

	float origins[3] = {
		ray.Origin.x,
		ray.Origin.y,
		ray.Origin.z
	};

	float invDirections[3] = {
		1.0f / ray.Direction.x,
		1.0f / ray.Direction.y,
		1.0f / ray.Direction.z
	};

	Vector3 v1;
	Vector3 v2;
	Vector3 v3;
	int cols = 0;

	stack.push_back(BIHStackData(this, sceneMin, sceneMax));

stackloop:
	while (stack.size() > 0)
	{
		BIHStackData data = stack[stack.size() - 1];
		stack.erase(stack.begin() + stack.size() - 1);
		BIHNode* node = data.Node;
		float tMin = data.Min, tMax = data.Max;
		if (tMax < tMin)
		{
			continue;
		}

	leafloop:
		while (node->Axis != 3)
		{
			int a = node->Axis;

			// find the origin and direction value for the given axis
			float origin = origins[a];
			float invDirection = invDirections[a];

			float tNearSplit, tFarSplit;
			BIHNode* nearNode;
			BIHNode* farNode;

			tNearSplit = (node->LeftPlane - origin) * invDirection;
			tFarSplit = (node->RightPlane - origin) * invDirection;
			nearNode = node->Left;
			farNode = node->Right;

			if (invDirection < 0)
			{
				float tmpSplit = tNearSplit;
				tNearSplit = tFarSplit;
				tFarSplit = tmpSplit;

				BIHNode* tmpNode = nearNode;
				nearNode = farNode;
				farNode = tmpNode;
			}

			if (tMin > tNearSplit && tMax < tFarSplit)
			{
				goto stackloop;
			}

			if (tMin > tNearSplit)
			{
				tMin = glm::max(tMin, tFarSplit);
				node = farNode;
			}
			else if (tMax < tFarSplit)
			{
				tMax = glm::min(tMax, tNearSplit);
				node = nearNode;
			}
			else
			{
				stack.push_back(BIHStackData(farNode, glm::max(tMin, tFarSplit), tMax));
				tMax = glm::min(tMax, tNearSplit);
				node = nearNode;
			}
		}

		// a leaf
		for (int i = node->LeftIndex; i <= node->RightIndex; i++)
		{
			tree->GetTriangle(i, v1, v2, v3);

			float t = 0;
			if (ray.IntersectWithTriangle(v1, v2, v3, t))
			{
				if (true)
				{
					v1 = worldMatrix * glm::vec4(v1, 1.0f);
					v2 = worldMatrix * glm::vec4(v2, 1.0f);
					v3 = worldMatrix * glm::vec4(v3, 1.0f);

					float t_world = 0;

					if (!(new Ray(o, d))->IntersectWithTriangle(v1, v2, v3, t_world))
					{
						continue;
					}
					t = t_world;
				}
				Vector3 contactNormal = Triangle::ComputeTriangleNormal(v1, v2, v3);
				Vector3 contactPoint = (Vector3(d) * t) + o;

				CollisionResult res;
				res.IsNull = false;
				res.ContactNormal = contactNormal;
				res.ContactPoint = contactPoint;
				res.Distance = glm::distance(o, contactPoint);
				res.TriangleIndex = tree->GetTriangleIndex(i);
				results.AddCollision(res);
				cols++;
			}
		}
	}
	stack.clear();
	return cols;
}