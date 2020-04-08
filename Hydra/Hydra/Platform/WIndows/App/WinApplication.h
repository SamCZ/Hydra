#pragma once

#include "Hydra/App/Application.h"

#include <Windows.h>

class WinWindow;
class MainRenderView;

class HYDRA_API WinApplication : public Application
{
private:
	HINSTANCE _hInstance;
	EngineContext* Context;
	MainRenderView* RenderView;
public:
	List<SharedPtr<WinWindow>> Windows;
public: // Inerhited from Application
	WinApplication();
	virtual ~WinApplication();

	virtual SharedPtr<FWindow> MakeWindow() override;
	virtual SharedPtr<FWindowRender> MakeWindowRenderer() override;

	virtual void InitializeWindow(const SharedPtr<FWindow>& InWindow, const SharedPtr<FWindowDefinition>& InDefinition, const SharedPtr<FWindow>& InParent) override;

	virtual void Run() override;

	virtual void InitializeEngineContext(EngineContext* context) override;

protected:
	virtual SharedPtr<FTaskbar> MakeTaskbar() override;
public:
	bool RegisterClassInstance(const HINSTANCE HInstance, const HICON HIcon);

	SharedPtr<WinWindow> FindWindowByHWND(HWND hwnd);
	int32 ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam);
public:
	String GetLastErrorAsString();
	void ThrowLastError(const String& title);

	EWindowTransparency GetWindowTransparencySupport() const;
};