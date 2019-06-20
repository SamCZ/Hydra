#pragma once

#include "Hydra/App/Taskbar.h"

struct ITaskbarList3;

class HYDRA_API  WinTaskbar : public FTaskbar
{
private:
	ITaskbarList3* TaskbarList3;
public:
	WinTaskbar();
	virtual ~WinTaskbar();

	virtual void SetOverlayIcon(const SharedPtr<FWindow>& NativeWindow, const FIcon& Icon, const String& Description);

	virtual void SetProgressState(const SharedPtr<FWindow>& NativeWindow, ETaskbarProgressState::Type State);

	virtual void SetProgressValue(const SharedPtr<FWindow>& NativeWindow, uint64 Current, uint64 Total);
};