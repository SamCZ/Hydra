#pragma once

#include "Hydra/Core/String.h"

namespace Hydra
{
	class Resource
	{
	protected:
		String _Source;
	public:

		void SetSource(const String& source);
		const String GetSource();
	};
}