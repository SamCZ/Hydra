#include "UIWidget.h"

UIWidget::UIWidget() :
	Parent(nullptr),
	ParentWindow(nullptr)
{

}

UIWidget::~UIWidget()
{
}

int32 UIWidget::OnPaint(FPaintRenderQueueLayered& paintQueue, UIRenderer& renderer, int layerID)
{
	return 0;
}

void UIWidget::SetParent(SharedPtr<UIWidget> widget)
{
	Parent = widget;

	if (widget != nullptr)
	{
		ParentWindow = widget->GetParentWindow();
	}
}

void UIWidget::SetParentWindow(SharedPtr<UIWindow> window)
{
	ParentWindow = window;
}

SharedPtr<UIWidget>& UIWidget::GetParent()
{
	return Parent;
}

SharedPtr<UIWindow>& UIWidget::GetParentWindow()
{
	return ParentWindow;
}
