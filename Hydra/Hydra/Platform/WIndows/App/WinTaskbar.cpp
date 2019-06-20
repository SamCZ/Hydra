#include "WinTaskbar.h"

#include <ShObjIdl.h> //Taskbar

#include "WinWindow.h"

WinTaskbar::WinTaskbar() : TaskbarList3(nullptr)
{
	::CoInitialize(NULL);

	if (CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, (void **)&TaskbarList3) != S_OK)
	{
		TaskbarList3 = nullptr;
	}
}

WinTaskbar::~WinTaskbar()
{
	if (TaskbarList3)
	{
		TaskbarList3->Release();
	}
}

void WinTaskbar::SetOverlayIcon(const SharedPtr<FWindow>& NativeWindow, const FIcon & Icon, const String & Description)
{
	if (TaskbarList3)
	{
		const SharedPtr<WinWindow> winWindow = StaticCastSharedPtr<WinWindow, FWindow>(NativeWindow);

		//TODO: Icon

		//TaskBarList3->SetOverlayIcon(Window->GetHWnd(), Icon, *Description.ToString());
	}
}

void WinTaskbar::SetProgressState(const SharedPtr<FWindow>& NativeWindow, ETaskbarProgressState::Type State)
{
	if (TaskbarList3)
	{
		const SharedPtr<WinWindow> winWindow = StaticCastSharedPtr<WinWindow, FWindow>(NativeWindow);

		TaskbarList3->SetProgressState(winWindow->GetHWnd(), (TBPFLAG)State);
	}
}

void WinTaskbar::SetProgressValue(const SharedPtr<FWindow>& NativeWindow, uint64 Current, uint64 Total)
{
	if (TaskbarList3)
	{
		const SharedPtr<WinWindow> winWindow = StaticCastSharedPtr<WinWindow, FWindow>(NativeWindow);

		TaskbarList3->SetProgressValue(winWindow->GetHWnd(), (ULONGLONG)Current, (ULONGLONG)Total);
	}
}
