#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Framework/Class.h"

#include "Object.generated.h"


HCLASS()
class HYDRA_API HObject
{
	HCLASS_GENERATED_BODY()
private:

public:
	String Name;
public:
	FORCEINLINE virtual ~HObject() { }

	template<class T>
	bool IsA() const
	{
		HClass clazz = GetClass();

		return clazz == T::StaticClass();
	}

	template<typename T>
	T* SafeCast()
	{
		if (IsA<T>())
		{
			return static_cast<T*>(this);
		}

		return nullptr;
	}

	template<typename T>
	const T* SafeConstCast() const
	{
		if (IsA<T>())
		{
			return static_cast<const T*>(this);
		}

		return nullptr;
	}

};