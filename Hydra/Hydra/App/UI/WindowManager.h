#pragma once

#include "Hydra/Core/Container.h"
#include "Hydra/App/UI/UIWindow.h"

class Application;

class HYDRA_API WindowManager
{
private:
	static WindowManager* SelfInstance;

	Application* App;

	List<SharedPtr<UIWindow>> Windows;
public:
	WindowManager(Application* app);
	~WindowManager();

	SharedPtr<UIWindow> AddWindow(SharedPtr<UIWindow>& window, bool showImmediately);

	static WindowManager& Get();

private:
	void MakeWindow(SharedPtr<UIWindow>& window);
};