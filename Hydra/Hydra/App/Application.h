#pragma once

#include "Hydra/Core/SmartPointer.h"
#include "Hydra/Core/Delegate.h"

#include "WindowManager.h"
#include "Window.h"
#include "Taskbar.h"

class FWindowRender;
class EngineContext;

class HYDRA_API Application
{
protected:
	WindowManager* IUWindowManager;
public:
	SharedPtr<FTaskbar> Taskbar;
	DelegateEvent<void, SharedPtr<FWindow>> OnWindowDestroy;
public:
	Application();
	virtual ~Application();

	void Initialize();

	virtual SharedPtr<FWindow> MakeWindow() { return MakeShared<FWindow>(); }

	virtual SharedPtr<FWindowRender> MakeWindowRenderer() { return nullptr; }

	virtual void InitializeWindow(const SharedPtr<FWindow>& InWindow, const SharedPtr<FWindowDefinition>& InDefinition, const SharedPtr<FWindow>& InParent);

	virtual void Run();

	virtual void InitializeEngineContext(EngineContext* context);

protected:
	virtual SharedPtr<FTaskbar> MakeTaskbar() { return MakeShared<FTaskbar>(); }
};