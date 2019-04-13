#pragma once

#include "Hydra/Physics/Collisons/Ray.h"
#include "Hydra/Physics/Collisons/CollisionResults.h"

namespace Hydra
{
	class BIHTree;

	class HYDRA_API BIHNode
	{
	public:
		int LeftIndex;
		int RightIndex;
		BIHNode* Left;
		BIHNode* Right;
		float LeftPlane;
		float RightPlane;
		int Axis;

		BIHNode(int l, int r);
		BIHNode(int axis);
		~BIHNode();

		BIHNode* GetLeftChild();
		void SetLeftChild(BIHNode* left);
		float GetLeftPlane();
		void SetLeftPlane(float leftPlane);
		BIHNode* GetRightChild();
		void SetRightChild(BIHNode* right);
		float GetRightPlane();
		void SetRightPlane(float rightPlane);

		int IntersectWhere(const Ray& r, const Matrix4& worldMatrix, BIHTree* tree, float sceneMin, float sceneMax, CollisionResults &results);
	};
}