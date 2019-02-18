#include "Resource.h"

namespace Hydra
{
	void Resource::SetSource(const String& source)
	{
		_Source = source;
	}

	const String Resource::GetSource()
	{
		return _Source;
	}
}