#include "Window.h"

FWindow::FWindow()
{
}

FWindow::~FWindow()
{
}

void FWindow::Show()
{
}

void FWindow::Hide()
{
}

void FWindow::Destroy()
{
}

void FWindow::Reshape(int32 NewX, int32 NewY, int32 NewWidth, int32 NewHeight)
{
}

void FWindow::Minimize()
{
}

void FWindow::Maximize()
{
}

void FWindow::Restore()
{
}

void FWindow::SetMode(EWindowMode::Type NewWindowMode)
{
}

EWindowMode::Type FWindow::GetMode() const
{
	return EWindowMode::Type::Windowed;
}

bool FWindow::IsMaximized() const
{
	return false;
}

bool FWindow::IsMinimized() const
{
	return false;
}

bool FWindow::IsVisible() const
{
	return false;
}

void FWindow::SetFocus()
{
}

void FWindow::SetOpacity(float Opacity)
{
}
