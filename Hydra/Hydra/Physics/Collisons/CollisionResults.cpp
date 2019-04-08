#include "Hydra/Physics/Collisons/CollisionResults.h"
#include "Hydra/Core/Sort.h"

namespace Hydra
{
	int compareCollisionResults(const CollisionResult left, const CollisionResult right)
	{
		return SORT_MIN_MAX(left.Distance, right.Distance);
	}

	CollisionResults::CollisionResults() : _results()
	{
	}

	void CollisionResults::Clear()
	{
		_results.clear();
	}

	void CollisionResults::AddCollision(CollisionResult result)
	{
		_results.push_back(result);
		_sorted = false;
	}

	int CollisionResults::Size()
	{
		return _results.size();
	}

	CollisionResult CollisionResults::GetClosestCollision()
	{
		if (Size() == 0)
		{
			return CollisionResult();
		}
		if (!_sorted)
		{
			MergeSort<CollisionResult>(_results, compareCollisionResults);
			_sorted = true;
		}
		return _results[0];
	}

	CollisionResult CollisionResults::GetFarthestCollision()
	{
		if (Size() == 0)
		{
			return CollisionResult();
		}
		if (!_sorted)
		{
			MergeSort<CollisionResult>(_results, compareCollisionResults);
			_sorted = true;
		}
		return _results[Size() - 1];
	}

	CollisionResult CollisionResults::GetCollision(int index)
	{
		if (Size() == 0)
		{
			return CollisionResult();
		}
		if (!_sorted)
		{
			MergeSort<CollisionResult>(_results, compareCollisionResults);
			_sorted = true;
		}
		return _results[index];
	}

	CollisionResult CollisionResults::GetCollisonDirect(int index)
	{
		if (Size() == 0)
		{
			return CollisionResult();
		}
		return _results[index];
	}
}