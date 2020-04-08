#include "Taskbar.h"

FTaskbar::FTaskbar()
{
}

FTaskbar::~FTaskbar()
{
}

void FTaskbar::SetOverlayIcon(const SharedPtr<FWindow>& NativeWindow, const FIcon & Icon, const String & Description)
{
}

void FTaskbar::SetProgressState(const SharedPtr<FWindow>& NativeWindow, ETaskbarProgressState::Type State)
{
}

void FTaskbar::SetProgressValue(const SharedPtr<FWindow>& NativeWindow, uint64 Current, uint64 Total)
{
}
