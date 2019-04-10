#pragma once

#include "Hydra/Core/Library.h"
#include "Hydra/Core/String.h"

namespace Hydra
{
	class HYDRA_EXPORT Resource
	{
	protected:
		String _Source;
	public:

		void SetSource(const String& source);
		const String GetSource();
	};
}