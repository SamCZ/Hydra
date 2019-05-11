#pragma once

#include "Hydra/Core/Library.h"
#include "Hydra/Core/String.h"
#include "Hydra/Core/Delegate.h"

class HObject;

template<typename T, typename From>
static T* Cast(From* from)
{
	return static_cast<T*>(from);
}


class HYDRA_API HClassDatabase
{
private:
	static int NextIndex;

	static Map<String, List<String>> RawClassDB;

	static FastMap<String, int> ClassIndexMap;
	static FastMap<int, List<int>> ClassHiearchy;
public:

	static int GetIndexOrGenerate(const String& name)
	{
		auto iter = ClassIndexMap.find(name);

		if (iter != ClassIndexMap.end())
		{
			return iter->second;
		}

		int index = NextIndex++;

		ClassIndexMap[name] = index;

		return index;
	}

	static void Add(const String& className, const List<String>& inheritedClasses)
	{
		RawClassDB[className] = inheritedClasses;
		
		int classIndex = GetIndexOrGenerate(className);

		if (ClassHiearchy.find(classIndex) != ClassHiearchy.end())
		{
			LogError("Class::Add", className, "Class already exists !");
			return;
		}

		for (const String& inheritedClass : inheritedClasses)
		{
			int inheritedClassIndex = GetIndexOrGenerate(inheritedClass);

			ClassHiearchy[classIndex].push_back(inheritedClassIndex);
		}

		// For game reflections

		if (inheritedClasses.size() > 0)
		{
			const String& lastInherited = inheritedClasses[inheritedClasses.size() - 1];

			if (lastInherited == "HObject") return;

			auto iter = RawClassDB.find(lastInherited);

			if (iter != RawClassDB.end())
			{
				for (String& otherInherited : iter->second)
				{
					int otherInheritedClassIndex = GetIndexOrGenerate(otherInherited);

					ClassHiearchy[classIndex].push_back(otherInheritedClassIndex);
				}
			}
		}
	}

	static bool IsInSameHiearchy(const String& left, const String& right)
	{
		int leftIndex = GetIndexOrGenerate(left);
		int rightIndex = GetIndexOrGenerate(right);

		if (leftIndex == rightIndex)
		{
			return true;
		}

		List<int>& map = ClassHiearchy[leftIndex];

		for (int dbIndex : map)
		{
			if (rightIndex == dbIndex)
			{
				return true;
			}
		}

		return false;
	}

};



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
		return HClassDatabase::IsInSameHiearchy(ClassName, left.ClassName);
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

#define HCLASS(...) 
#define HCLASS_GENERATED_BODY(...)

#define HENUM(...)
#define INLINE_GENERATE_ENUM_TO_STRING(...)

/*
#define HCLASS_BODY(Name) protected: \
							  static HObject* Factory_##Name() { return new Name(); } \
							  public: \
							  static HClass StaticClass() { return HClass(#Name, Name::Factory_##Name); } \
							  virtual HClass GetClass() const { return HClass(#Name, Name::Factory_##Name); }
*/
#define HCLASS_BODY_NO_FNC_POINTER(Name) public: \
							  static HClass StaticClass() { return HClass(#Name, nullptr); } \
							  virtual HClass GetClass() const { return HClass(#Name, nullptr); }