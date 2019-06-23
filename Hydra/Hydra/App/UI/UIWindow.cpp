#include "UIWindow.h"

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

	bCreateTitleBar = InArgs._CreateTitleBar && !bIsPopupWindow && Type != EWindowType::CursorDecorator && !bHasOSWindowBorder;

	Size = Vector2i(800, 600);
	ScreenPosition = Vector2i(200, 200);
}

void UIWindow::SetNativeWindow(SharedPtr<FWindow>& window)
{
	NativeWindow = window;
}

SharedPtr<FWindow>& UIWindow::GetNativeWindow()
{
	return NativeWindow;
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

		//TODO: Viewport initialization
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
