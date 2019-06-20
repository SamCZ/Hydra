#include "Application.h"

Application::Application()
{
	Taskbar = MakeTaskbar();
}

Application::~Application()
{
}

void Application::InitializeWindow(const SharedPtr<FWindow>& InWindow, const SharedPtr<FWindowDefinition>& InDefinition, const SharedPtr<FWindow>& InParent, const bool bShowImmediately)
{
}
