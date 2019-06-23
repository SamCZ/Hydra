#include "Application.h"

Application::Application()
{
	Taskbar = MakeTaskbar();
	IUWindowManager = new WindowManager(this);
}

Application::~Application()
{
	delete IUWindowManager;
}

void Application::InitializeWindow(const SharedPtr<FWindow>& InWindow, const SharedPtr<FWindowDefinition>& InDefinition, const SharedPtr<FWindow>& InParent)
{
}

void Application::Run()
{
}
