#include "Application.h"

Application::Application()
{

}

Application::~Application()
{
	delete IUWindowManager;
}

void Application::Initialize()
{
	Taskbar = MakeTaskbar();
	IUWindowManager = new WindowManager(this);
}

void Application::InitializeWindow(const SharedPtr<FWindow>& InWindow, const SharedPtr<FWindowDefinition>& InDefinition, const SharedPtr<FWindow>& InParent)
{
}

void Application::Run()
{
}

void Application::InitializeEngineContext(EngineContext * context)
{
}
