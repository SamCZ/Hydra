#include "ViewPort.h"

FViewPort::FViewPort() : _Width(0), _Height(0), _IsSizeValid(false)
{
}

FViewPort::FViewPort(int width, int height) : _Width(width), _Height(height), _IsSizeValid(false)
{
	if (width > 0 && height > 0)
	{
		_IsSizeValid = true;
	}
}

void FViewPort::Resize(int width, int height)
{
	_Width = width;
	_Height = height;

	_OnResizeEvent.Invoke(width, height);
}

int FViewPort::GetWidth() const
{
	return _Width;
}

int FViewPort::GetHeight() const
{
	return _Height;
}

bool FViewPort::IsValid() const
{
	return _IsSizeValid;
}

void FViewPort::AddResizeListener(const NammedDelegate<void, int, int>& Event)
{
	_OnResizeEvent += Event;
}

void FViewPort::RemoveListener(const String & EventName)
{
	_OnResizeEvent -= EventName;
}
