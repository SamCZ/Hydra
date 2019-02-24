#pragma once

#include <memory>

namespace Hydra
{
	template<typename T>
	using SharedPtr = std::shared_ptr<T>;

	template<typename T>
	using WeakPtr = std::weak_ptr<T>;

	template<typename T, typename... _Types>
	inline static SharedPtr<T> MakeShared(_Types&&... _Args)
	{
		return std::make_shared<T>(_Args...);
	}

	//#define New(classname) MakeShared<classname>()
	#define New(classname, ...) MakeShared<classname>(__VA_ARGS__)

	#define DEFINE_PTR(className) typedef SharedPtr<className> className ## Ptr; typedef WeakPtr<className> className ## WeakPtr;
}