#pragma once

#include <functional>

namespace Hydra
{
	template <typename T>
	using Function = std::function<T>;
}