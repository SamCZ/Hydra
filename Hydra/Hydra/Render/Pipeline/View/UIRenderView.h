#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Core/Delegate.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

class HYDRA_API HydraEngine;
class HYDRA_API EngineContext;

class UIRenderView
{
public:
	HydraEngine* Engine;
	EngineContext* Context;

	UIRenderView(EngineContext* context, HydraEngine* engine) : Context(context), Engine(engine) { }

	void OnCreated();
	void OnDestroy();

	void OnRender(NVRHI::TextureHandle mainRenderTarget);
	void OnTick(float Delta);
	void OnResize(uint32 width, uint32 height, uint32 sampleCount);
};