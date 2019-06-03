#pragma once

#include "Hydra/Core/Delegate.h"
#include "Hydra/Render/View/SceneView.h"

class FViewPort
{
private:
	int _Width;
	int _Height;
	bool _IsSizeValid;

	FSceneView* _SceneView;

	DelegateEvent<void, int, int> _OnResizeEvent;
public:
	FViewPort();
	FViewPort(int width, int height);

	void SetSceneView(FSceneView* view);
	FSceneView* GetSceneView();

	void Resize(int width, int height);

	int GetWidth() const;
	int GetHeight() const;

	bool IsValid() const;

	void AddResizeListener(const NammedDelegate<void, int, int>& Event);
	void RemoveListener(const String& EventName);
};