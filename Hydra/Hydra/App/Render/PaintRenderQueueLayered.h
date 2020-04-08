#pragma once

#include "Hydra/Core/Container.h"
#include "Hydra/Core/Function.h"

class FPaintRenderQueueLayered
{
private:
	List<Pair<int, Function<void()>>> RenderFunctions;
public:
	FPaintRenderQueueLayered();
	~FPaintRenderQueueLayered();

	void Add(int layerID, Function<void()> renderFunction);

	void Render();
};