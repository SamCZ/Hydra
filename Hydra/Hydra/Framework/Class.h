#pragma once

#include "Hydra/Core/Library.h"
#include "Hydra/Core/String.h"
#include "Hydra/Core/Delegate.h"

class HObject;

template<typename T, typename From>
FORCEINLINE static T* Cast(From* from)
{
	return static_cast<T*>(from);
}

class HYDRA_API HClass
{
private:
	String ClassName;
	FUNC_POINTER(Factory, HObject);

public:
	HClass(const String& className, FUNC_POINTER(factory, HObject)) : ClassName(className), Factory(factory)
	{

	}

	bool operator==(const HClass& left)
	{
		return ClassName == left.ClassName;
	}

	String GetName() const
	{
		return ClassName;
	}

	HObject* CreateInstance()
	{
		return Factory();
	}

	template<typename HObject>
	HObject* CreateInstance()
	{
		return static_cast<HObject*>(Factory());
	}
};

#define HCLASS_BODY(Name) protected: \
							  static HObject* Factory_##Name() { return new Name(); } \
							  public: \
							  static HClass StaticClass() { return HClass(#Name, Name::Factory_##Name); } \
							  virtual HClass GetClass() const { return HClass(#Name, Name::Factory_##Name); }

#define HCLASS_BODY_NO_FNC_POINTER(Name) public: \
							  static HClass StaticClass() { return HClass(#Name, nullptr); } \
							  virtual HClass GetClass() const { return HClass(#Name, nullptr); }