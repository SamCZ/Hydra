#pragma once

#include "Hydra/Render/Pipeline/DeviceManager.h"

class HYDRA_API HydraEngine;
class HYDRA_API HSceneComponent;

class MainRenderView : public IVisualController
{
public:
	HydraEngine* Engine;

	MainRenderView(EngineContext* context, HydraEngine* engine) : IVisualController(context), Engine(engine) {}

	void OnCreated();
	void OnDestroy();

	void OnRender(NVRHI::TextureHandle mainRenderTarget);
	void OnTick(float Delta);
	void OnResize(uint32 width, uint32 height, uint32 sampleCount);

private:
	void UpdateComponent(HSceneComponent* component, float Delta);
};