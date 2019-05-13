#include "Resource.h"

void Resource::SetSource(const String& source)
{
	_Source = source;
}

const String Resource::GetSource()
{
	return _Source;
}