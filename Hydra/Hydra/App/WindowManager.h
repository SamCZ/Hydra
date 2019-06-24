#pragma once

#include "Hydra/Core/Container.h"
#include "Hydra/App/UI/UIWindow.h"
#include "Hydra/App/Render/WindowRender.h"

class Application;
class FWindow;

class HYDRA_API WindowManager
{
private:
	static WindowManager* SelfInstance;

	Application* App;
	SharedPtr<FWindowRender> WindowRenderer;
	List<SharedPtr<UIWindow>> Windows;
public:
	WindowManager(Application* app);
	~WindowManager();

	void Tick(float Delta);
	void Render();

	SharedPtr<UIWindow> AddWindow(SharedPtr<UIWindow>& window, bool showImmediately);

	FWindowRender& GetRenderer();

	static WindowManager& Get();
private:
	void MakeWindow(SharedPtr<UIWindow>& window);
	void OnWindowDestroy(SharedPtr<FWindow> nativeWindow);
};