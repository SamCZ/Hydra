#pragma once

#include "Hydra/Core/Vector.h"
#include "Hydra/Core/Container.h"

namespace Hydra
{
	class Spatial;

	struct CollisionResult
	{
		bool IsNull = true;
		Spatial* Obj;
		Vector3 ContactPoint;
		Vector3 ContactNormal;
		float Distance;
		int TriangleIndex;
	};

	class CollisionResults
	{
	public:
		CollisionResults();
		void Clear();
		void AddCollision(CollisionResult result);
		int Size();
		CollisionResult GetClosestCollision();
		CollisionResult GetFarthestCollision();
		CollisionResult GetCollision(int index);
		CollisionResult GetCollisonDirect(int index);
	private:
		List<CollisionResult> _results;
		bool _sorted = false;
	};
}