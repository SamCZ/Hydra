#pragma once

#include "Hydra/Core/SmartPointer.h"
#include "Hydra/Core/Delegate.h"
#include "Hydra/App/UI/WindowManager.h"

#include "Window.h"
#include "Taskbar.h"

class HYDRA_API Application
{
private:
	WindowManager* IUWindowManager;
public:
	SharedPtr<FTaskbar> Taskbar;
	DelegateEvent<void, SharedPtr<FWindow>> OnWindowDestroy;
public:
	Application();
	virtual ~Application();

	virtual SharedPtr<FWindow> MakeWindow() { return MakeShared<FWindow>(); }

	virtual void InitializeWindow(const SharedPtr<FWindow>& InWindow, const SharedPtr<FWindowDefinition>& InDefinition, const SharedPtr<FWindow>& InParent);

	virtual void Run();

protected:
	virtual SharedPtr<FTaskbar> MakeTaskbar() { return MakeShared<FTaskbar>(); }
};