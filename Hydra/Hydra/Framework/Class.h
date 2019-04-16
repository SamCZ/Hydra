#pragma once

#include "Hydra/Core/String.h"

namespace Hydra
{
	class HClass
	{
	private:
		String ClassName;

	public:
		HClass(const String& className) : ClassName(className)
		{

		}

		bool operator==(const HClass& left)
		{
			return ClassName == left.ClassName;
		}

		String GetClass() const
		{
			return ClassName;
		}
	};

	#define HCLASS_BODY(name) 
}