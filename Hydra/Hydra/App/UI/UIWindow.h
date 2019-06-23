#pragma once

#include "UI.h"
#include "UIWidget.h"
#include "Hydra/Core/Vector.h"
#include "Hydra/App/Window.h"
#include "Hydra/Framework/Optional.h"

enum class ESizingRule : uint8
{
	/* The windows size fixed and cannot be resized **/
	FixedSize,

	/** The window size is computed from its content and cannot be resized by users */
	Autosized,

	/** The window can be resized by users */
	UserSized,
};

class HYDRA_API UIWindow : public UIWidget
{
public:
	UI_BEGIN_ARGS(UIWindow)
		: _Type(EWindowType::Normal)
		, _Title()
		, _AutoCenter(true)
		, _ScreenPosition()
		, _Size()
		, _SupportsTransparency(EWindowTransparency::None)
		, _InitialOpacity(1.0f)
		, _IsInitiallyMaximized(false)
		, _IsInitiallyMinimized(false)
		, _SizingRule(ESizingRule::UserSized)
		, _IsPopupWindow(false)
		, _IsTopmostWindow(false)
		, _FocusWhenFirstShown(true)
		, _UseOSWindowBorder(false)
		, _HasCloseButton(true)
		, _SupportsMaximize(true)
		, _SupportsMinimize(true)
		, _ShouldPreserveAspectRatio(false)
		, _CreateTitleBar(true)
		, _SaneWindowPlacement(true)
	{

	}

	UI_ARGUMENT(EWindowType, Type);
	//TODO: Styles
	UI_ARGUMENT(String, Title);
	UI_ARGUMENT(bool, AutoCenter);
	UI_ARGUMENT(Vector2i, ScreenPosition);
	UI_ARGUMENT(Vector2i, Size);
	UI_ARGUMENT(EWindowTransparency, SupportsTransparency);
	UI_ARGUMENT(float, InitialOpacity);
	UI_ARGUMENT(bool, IsInitiallyMaximized);
	UI_ARGUMENT(bool, IsInitiallyMinimized);
	UI_ARGUMENT(ESizingRule, SizingRule);
	UI_ARGUMENT(bool, IsPopupWindow);
	UI_ARGUMENT(bool, IsTopmostWindow);
	UI_ARGUMENT(bool, FocusWhenFirstShown);
	UI_ARGUMENT(EWindowActivationPolicy, ActivationPolicy);
	UI_ARGUMENT(bool, UseOSWindowBorder);
	UI_ARGUMENT(bool, HasCloseButton);
	UI_ARGUMENT(bool, SupportsMaximize);
	UI_ARGUMENT(bool, SupportsMinimize);
	UI_ARGUMENT(bool, ShouldPreserveAspectRatio);
	UI_ARGUMENT(TOptional<float>, MinWidth);
	UI_ARGUMENT(TOptional<float>, MinHeight);
	UI_ARGUMENT(TOptional<float>, MaxWidth);
	UI_ARGUMENT(TOptional<float>, MaxHeight);
	UI_ARGUMENT(bool, CreateTitleBar);
	UI_ARGUMENT(bool, SaneWindowPlacement);

	UI_DEFAULT_SLOT(FArguments, Content);

	UI_END_ARGS;

private:
	SharedPtr<FWindow> NativeWindow;

protected:
	EWindowType Type;
	String Title;
	bool bDragAnywhere;
	float Opacity;
	ESizingRule SizingRule;
	EWindowTransparency TransparencySupport;
	bool bAutoCenter;
	bool bCreateTitleBar : 1;
	bool bIsPopupWindow : 1;
	bool bIsTopmostWindow : 1;
	bool bSizeWillChangeOften : 1;
	bool bInitiallyMaximized : 1;
	bool bInitiallyMinimized : 1;
	bool bHasEverBeenShown : 1;
	bool bFocusWhenFirstShown : 1;
	bool bHasOSWindowBorder : 1;
	bool bVirtualWindow : 1;
	bool bHasCloseButton : 1;
	bool bHasMinimizeButton : 1;
	bool bHasMaximizeButton : 1;
	bool bHasSizingFrame : 1;
	bool bIsModalWindow : 1;
	bool bIsMirrorWindow : 1;
	bool bShouldPreserveAspectRatio : 1;
	EWindowActivationPolicy WindowActivationPolicy;
	Vector2i ScreenPosition;
	Vector2i Size;

	//Vector2i ViewportSize;
	//TWeakPtr<ISlateViewport> Viewport;

public:
	virtual ~UIWindow();

	void Initialize(const FArguments& arguments);

	void SetNativeWindow(SharedPtr<FWindow>& window);
	SharedPtr<FWindow>& GetNativeWindow();

	bool SupportsKeyboardFocus() const;
	bool IsFocusedInitially() const;

	void Show();
	void SetFocus();

public:

	inline const EWindowType& GetType() const { return Type; }
	inline const Vector2i& GetSize() const { return Size; }
	inline const Vector2i& GetScreenPosition() const { return ScreenPosition; }
	inline const bool HasOSWindowBorder() const { return bHasOSWindowBorder; }
	inline const EWindowTransparency& GetTransparencySupport() const { return TransparencySupport; }
	inline const bool AppearsInTaskbar() const { return !bIsPopupWindow && Type != EWindowType::ToolTip && Type != EWindowType::CursorDecorator; }
	inline const bool IsTopmostWindow() const { return bIsTopmostWindow; }
	inline const bool AcceptsInput() const { return Type != EWindowType::CursorDecorator && Type != EWindowType::ToolTip; }
	inline const EWindowActivationPolicy& ActivationPolicy() const { return WindowActivationPolicy; }
	inline const bool HasCloseBox() const { return bHasCloseButton; }
	inline const bool HasMinimizeBox() const { return bHasMinimizeButton; }
	inline const bool HasMaximizeBox() const { return bHasMaximizeButton; }
	inline const bool IsModalWindow() const { return bIsModalWindow; }
	inline const bool IsRegularWindow() const { return !bIsPopupWindow && Type != EWindowType::ToolTip && Type != EWindowType::CursorDecorator; }
	inline const bool HasSizingFrame() const { return bHasSizingFrame; }
	inline const bool SizeWillChangeOften() const { return bSizeWillChangeOften; }
	inline const bool ShouldPreserveAspectRatio() const { return bShouldPreserveAspectRatio; }
	inline const String& GetTitle() const { return Title; }
	inline const float GetOpacity() const { return Opacity; }


};