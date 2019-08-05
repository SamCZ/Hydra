#include "UIWindow.h"

#include "Hydra/App/WindowManager.h"

UIWindow::UIWindow() : NativeWindow(nullptr), ViewPort(nullptr), bHasEverBeenShown(false), bSizeWillChangeOften(false)
{
}

UIWindow::~UIWindow()
{
}

void UIWindow::Initialize(const FArguments& InArgs)
{
	this->Type = InArgs._Type;

	this->Title = InArgs._Title;
	this->bDragAnywhere = true;
	this->TransparencySupport = InArgs._SupportsTransparency;
	this->Opacity = InArgs._InitialOpacity;
	this->bInitiallyMaximized = InArgs._IsInitiallyMaximized;
	this->bInitiallyMinimized = InArgs._IsInitiallyMinimized;
	this->SizingRule = InArgs._SizingRule;
	this->bIsPopupWindow = InArgs._IsPopupWindow;
	this->bIsTopmostWindow = InArgs._IsTopmostWindow;
	this->bFocusWhenFirstShown = InArgs._FocusWhenFirstShown;
	this->bHasOSWindowBorder = InArgs._UseOSWindowBorder;
	this->bHasCloseButton = InArgs._HasCloseButton;
	this->bHasMinimizeButton = InArgs._SupportsMinimize;
	this->bHasMaximizeButton = InArgs._SupportsMaximize;
	this->bHasSizingFrame = !InArgs._IsPopupWindow && InArgs._SizingRule == ESizingRule::UserSized;
	this->bShouldPreserveAspectRatio = InArgs._ShouldPreserveAspectRatio;
	this->WindowActivationPolicy = InArgs._ActivationPolicy;
	this->bVirtualWindow = false;

	Size = InArgs._Size;
	ScreenPosition = InArgs._ScreenPosition;

	bCreateTitleBar = InArgs._CreateTitleBar && !bIsPopupWindow && Type != EWindowType::CursorDecorator && !bHasOSWindowBorder;

	if (SizingRule == ESizingRule::Autosized || (Size.x == 0 || Size.y == 0))
	{
		Size = Vector2i(1280, 720);
	}

	if (InArgs._AutoCenter)
	{
		ScreenPosition = Vector2i(1920, 1080) / 2 - (Size / 2);
	}

	ContentWidget = InArgs._Content;
}

void UIWindow::SetNativeWindow(SharedPtr<FWindow>& window)
{
	NativeWindow = window;
}

SharedPtr<FWindow>& UIWindow::GetNativeWindow()
{
	return NativeWindow;
}

void UIWindow::SetViewPort(SharedPtr<FWindowViewPort>& viewport)
{
	ViewPort = viewport;
}

SharedPtr<FWindowViewPort> UIWindow::GetViewport()
{
	return ViewPort;
}

bool UIWindow::SupportsKeyboardFocus() const
{
	return Type != EWindowType::ToolTip && Type != EWindowType::CursorDecorator;
}

bool UIWindow::IsFocusedInitially() const
{
	return bFocusWhenFirstShown;
}

void UIWindow::Show()
{
	if (!bHasEverBeenShown)
	{
		bHasEverBeenShown = true;

		WindowManager::Get().GetRenderer().CreateViewport(AsShared<UIWindow>());

		if (NativeWindow != nullptr && bInitiallyMaximized)
		{
			NativeWindow->Maximize();
		}

		if (NativeWindow != nullptr && bInitiallyMinimized)
		{
			NativeWindow->Minimize();
		}
	}

	if (NativeWindow != nullptr)
	{
		NativeWindow->Show();
	}
}

void UIWindow::SetFocus()
{
	if (NativeWindow != nullptr)
	{
		NativeWindow->SetFocus();
	}
}

int32 UIWindow::OnPaint(FPaintRenderQueueLayered & paintQueue, UIRenderer & renderer, int layerID)
{
	return ContentWidget->OnPaint(paintQueue, renderer, layerID + 1);
}
