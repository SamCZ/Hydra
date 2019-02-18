#pragma once

#include <vector>
#include <map>

namespace Hydra
{
	template <typename A>
	using List = std::vector<A>;

	template <typename A, typename B>
	using Map = std::map<A, B>;

	#define List_Remove(list, type) { list.erase(std::find(list.begin(), list.end(), type)); }
}