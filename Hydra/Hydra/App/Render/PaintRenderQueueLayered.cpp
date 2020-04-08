#include "PaintRenderQueueLayered.h"

#include "Hydra/Core/Sort.h"

FPaintRenderQueueLayered::FPaintRenderQueueLayered()
{
}

FPaintRenderQueueLayered::~FPaintRenderQueueLayered()
{
}

void FPaintRenderQueueLayered::Add(int layerID, Function<void()> renderFunction)
{
	RenderFunctions.push_back(Pair<int, Function<void()>>(layerID, renderFunction));
}

int SortRenderLayers(Pair<int, Function<void()>>& left, Pair<int, Function<void()>>& right)
{
	if (left.first > right.first)
	{
		return 1;
	}
	else
	{
		return -1;
	}

	return 0;
}

void FPaintRenderQueueLayered::Render()
{
	MergeSort<Pair<int, Function<void()>>>(RenderFunctions, SortRenderLayers);

	for (Pair<int, Function<void()>> pair : RenderFunctions)
	{
		pair.second();
	}

	RenderFunctions.clear();
}
