#pragma once

#include "Hydra/Core/Vector.h"
#include "Hydra/Physics/Collisons/Ray.h"
#include "Hydra/Physics/Collisons/CollisionResults.h"

namespace Hydra
{
	class BIHNode;
	class Box;
	class Mesh;

	constexpr int MAX_BIH_SWAP_TMP = 9;

	class BIHTree
	{
	private:
		const int MAX_TREE_DEPTH = 100;
		const int MAX_TRIS_PER_NODE = 21;
		
		int _MaxTrisPerNode;

		int _NumTris;
		int _NumPointData;
		int _NumTriIndices;

		float* _PointData;
		int* _TriIndices;

		float _BihSwapTmp[MAX_BIH_SWAP_TMP];

		CollisionResults _BBoxTempRes;

		BIHNode* _Root;

	public:
		BIHTree(Mesh* mesh, int maxTrisPerNode);
		BIHTree(Mesh* mesh);
		~BIHTree();

		void GetTriangle(int index, Vector3 &v1, Vector3 &v2, Vector3 &v3);
		int GetTriangleIndex(int triIndex);

		int CollideWithRay(const Ray& r, const Matrix4& worldMatrix, Box* worldBound, CollisionResults &results);
	private:
		Box CreateBox(int l, int r);
		BIHNode* CreateNode(int l, int r, const Box& nodeBbox, int depth);

		void SetMinMax(Box& bbox, bool doMin, int axis, float value);
		float GetMinMax(Box& bbox, bool doMin, int axis);

		int SortTriangles(int l, int r, float split, int axis);
		void SwapTriangles(int index1, int index2);
		void ArrayCopy(float* src, int srcPos, float* dest, int destPos, int length);
	private:
		void InitTriangles(Mesh* mesh);
		void ConstructRootNode();
	};
}