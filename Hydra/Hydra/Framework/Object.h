#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Framework/Class.h"

namespace Hydra
{
	class HYDRA_API HObject
	{
		HCLASS_BODY(HObject)
	private:

	public:
		String Name;

		template<class T>
		bool IsA()
		{
			return GetClass() == T::StaticClass();
		}
	};
}