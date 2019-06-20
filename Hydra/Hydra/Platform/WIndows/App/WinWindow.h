#pragma once

#include "Hydra/App/Window.h"

#include <Windows.h>

class HYDRA_API WinWindow : public FWindow
{
private:
	SharedPtr<FWindowDefinition> Definition;

	HWND HWnd;

	EWindowMode::Type WindowMode;

	int32 RegionWidth;
	int32 RegionHeight;

	int32 VirtualWidth;
	int32 VirtualHeight;

	float AspectRatio;

	bool bIsVisible : 1;
	bool bIsFirstTimeVisible : 1;
	bool bInitiallyMinimized : 1;
	bool bInitiallyMaximized : 1;

	WINDOWPLACEMENT PreFullscreenWindowPlacement;
	WINDOWPLACEMENT PreParentMinimizedWindowPlacement;
protected:
	WinWindow();
public:
	virtual ~WinWindow();

	void Initialize(class WinApplication* const Application, const SharedPtr<FWindowDefinition>& InDefinition, HINSTANCE InHInstance, const SharedPtr<WinWindow>& InParent);

	HWND GetHWnd();

	static SharedPtr<WinWindow> Make();

	bool IsRegularWindow() const;

	void AdjustWindowRegion(int32 Width, int32 Height);
	HRGN MakeWindowRegionObject(bool bIncludeBorderWhenMaximized) const;
	int32 GetWindowBorderSize() const;

public: // Getters
	FWindowDefinition& GetDefinition() const;

public: // Inherited from FWindow

	virtual void Show();
	virtual void Hide();

	virtual void Destroy();

	virtual void Reshape(int32 NewX, int32 NewY, int32 NewWidth, int32 NewHeight);

	virtual void Minimize();
	virtual void Maximize();
	virtual void Restore();

	virtual void SetMode(EWindowMode::Type NewWindowMode) override;

	virtual EWindowMode::Type GetMode() const;

	virtual bool IsMaximized() const;
	virtual bool IsMinimized() const;

	virtual bool IsVisible() const;

	virtual void SetFocus();

	virtual void SetOpacity(float Opacity);
};