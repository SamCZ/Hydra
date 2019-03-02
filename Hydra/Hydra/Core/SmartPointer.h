#pragma once

#include <memory>
#include <type_traits>

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

	template<typename T>
	inline static SharedPtr<T> MakeShareable(T* pointer)
	{
		return SharedPtr<T> { pointer };
	}

	#define New(classname, ...) MakeShared<classname>(__VA_ARGS__)

	#define DEFINE_PTR(className) typedef SharedPtr<className> className ## Ptr; typedef WeakPtr<className> className ## WeakPtr;

	#define T_IS(tName, type) std::is_same<tName, type>::value
}