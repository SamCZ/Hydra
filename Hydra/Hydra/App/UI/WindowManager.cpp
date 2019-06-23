#include "WindowManager.h"

#include "Hydra/App/Application.h"

WindowManager* WindowManager::SelfInstance = nullptr;

WindowManager::WindowManager(Application* app) : App(app)
{
	SelfInstance = this;
}

WindowManager::~WindowManager()
{
	SelfInstance = nullptr;
}

SharedPtr<UIWindow> WindowManager::AddWindow(SharedPtr<UIWindow>& window, bool showImmediately)
{
	Windows.push_back(window);

	MakeWindow(window);

	if (showImmediately)
	{
		window->Show();

		if (window->SupportsKeyboardFocus() && window->IsFocusedInitially())
		{
			window->SetFocus();
		}
	}

	return window;
}

WindowManager& WindowManager::Get()
{
	return *SelfInstance;
}

void WindowManager::MakeWindow(SharedPtr<UIWindow>& InSlateWindow)
{
	SharedPtr<FWindowDefinition> Definition = MakeShared<FWindowDefinition>();

	Definition->Type = InSlateWindow->GetType();

	const Vector2i Size = InSlateWindow->GetSize();
	Definition->WidthDesiredOnScreen = Size.x;
	Definition->HeightDesiredOnScreen = Size.y;

	const Vector2i Position = InSlateWindow->GetScreenPosition();
	Definition->XDesiredPositionOnScreen = Position.x;
	Definition->YDesiredPositionOnScreen = Position.y;

	Definition->HasOSWindowBorder = InSlateWindow->HasOSWindowBorder();
	Definition->TransparencySupport = InSlateWindow->GetTransparencySupport();
	Definition->AppearsInTaskbar = InSlateWindow->AppearsInTaskbar();
	Definition->IsTopmostWindow = InSlateWindow->IsTopmostWindow();
	Definition->AcceptsInput = InSlateWindow->AcceptsInput();
	Definition->ActivationPolicy = InSlateWindow->ActivationPolicy();
	Definition->FocusWhenFirstShown = InSlateWindow->IsFocusedInitially();

	Definition->HasCloseButton = InSlateWindow->HasCloseBox();
	Definition->SupportsMinimize = InSlateWindow->HasMinimizeBox();
	Definition->SupportsMaximize = InSlateWindow->HasMaximizeBox();

	Definition->IsModalWindow = InSlateWindow->IsModalWindow();
	Definition->IsRegularWindow = InSlateWindow->IsRegularWindow();
	Definition->HasSizingFrame = InSlateWindow->HasSizingFrame();
	Definition->SizeWillChangeOften = InSlateWindow->SizeWillChangeOften();
	Definition->ShouldPreserveAspectRatio = InSlateWindow->ShouldPreserveAspectRatio();
	//Definition->ExpectedMaxWidth = InSlateWindow->GetExpectedMaxWidth();
	//Definition->ExpectedMaxHeight = InSlateWindow->GetExpectedMaxHeight();

	Definition->Title = InSlateWindow->GetTitle();
	Definition->Opacity = InSlateWindow->GetOpacity();
	//Definition->CornerRadius = InSlateWindow->GetCornerRadius();

	SharedPtr<FWindow> nativeWindow = App->MakeWindow();
	App->InitializeWindow(nativeWindow, Definition, nullptr);

	InSlateWindow->SetNativeWindow(nativeWindow);
}
