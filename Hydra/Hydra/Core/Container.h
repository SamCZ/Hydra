#pragma once

#include <vector>
#include <map>
#include <unordered_map>

namespace Hydra
{
	template <typename A>
	using List = std::vector<A>;

	template <typename A, typename B>
	using Map = std::map<A, B>;

	template <typename A, typename B>
	using FastMap = std::unordered_map<A, B>;

	#define List_Remove(list, type) { list.erase(std::find(list.begin(), list.end(), type)); }
	//#define Add(type) push_back(type)

	#define ITER(map, it_name) for(decltype(map)::iterator it_name = map.begin(); it_name != map.end(); it_name++)
}