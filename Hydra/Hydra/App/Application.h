#pragma once

#include "Hydra/Core/SmartPointer.h"
#include "Window.h"
#include "Taskbar.h"

class HYDRA_API Application
{
public:
	SharedPtr<FTaskbar> Taskbar;
public:
	Application();
	virtual ~Application();

	virtual SharedPtr<FWindow> MakeWindow() { return MakeShared<FWindow>(); }

	virtual void InitializeWindow(const SharedPtr<FWindow>& InWindow, const SharedPtr<FWindowDefinition>& InDefinition, const SharedPtr<FWindow>& InParent, const bool bShowImmediately);

protected:
	virtual SharedPtr<FTaskbar> MakeTaskbar() { return MakeShared<FTaskbar>(); }
};