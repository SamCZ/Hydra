#pragma once

#include "Hydra/Render/Pipeline/DeviceManager.h"

class HydraEngine;
class HSceneComponent;
class HCameraComponent;
class FSceneView;

class MaterialInterface;

class MainRenderView : public IVisualController
{
private:
	MaterialInterface* _DefaultMaterial;

	Map<HCameraComponent*, FSceneView*> _SceneViewForCameras;
public:
	HydraEngine* Engine;

	MainRenderView(EngineContext* context, HydraEngine* engine) : IVisualController(context), Engine(engine) {}

	void OnCreated();
	void OnDestroy();

	void OnRender(NVRHI::TextureHandle mainRenderTarget);
	void OnTick(float Delta);
	void OnResize(uint32 width, uint32 height, uint32 sampleCount);

private:
	void OnCameraAdded(HCameraComponent* cmp);
	void OnCameraRemoved(HCameraComponent* cmp);

private:
	void UpdateComponent(HSceneComponent* component, float Delta);

	void RenderSceneViewFromCamera(FSceneView* view, HCameraComponent* camera);
};