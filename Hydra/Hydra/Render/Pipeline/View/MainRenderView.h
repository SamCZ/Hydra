#pragma once

#include "Hydra/Render/Pipeline/DeviceManager.h"

class HydraEngine;
class HPrimitiveComponent;
class HSceneComponent;
class HCameraComponent;
class HStaticMesh;
class FSceneView;
class FViewPort;

class MaterialInterface;

class MainRenderView : public IVisualController
{
private:
	MaterialInterface* _DefaultMaterial;

	Map<HCameraComponent*, FSceneView*> _SceneViewForCameras;
	FViewPort* _ScreenRenderViewport;

	Map<uint32, NVRHI::InputLayoutHandle> _InputLayoutMap;
	Map<String, uint32> _InputLayoutHashID;
	uint32 _InputLayoutMaxID;

public:
	HydraEngine* Engine;

	MainRenderView(EngineContext* context, HydraEngine* engine);
	~MainRenderView();

	void OnCreated();
	void OnDestroy();

	void OnRender(NVRHI::TextureHandle mainRenderTarget);
	void OnTick(float Delta);
	void OnResize(uint32 width, uint32 height, uint32 sampleCount);

private:
	void OnCameraAdded(HCameraComponent* cmp);
	void OnCameraRemoved(HCameraComponent* cmp);

	void OnMeshLoaded(HStaticMesh* mesh);
	void OnMeshDeleted(HStaticMesh* mesh);

private:
	NVRHI::InputLayoutHandle GetInputLayoutForMaterial(MaterialInterface* materialInterface);

private:
	void UpdateMaterialGlobalVariables(MaterialInterface* materialInterface, HPrimitiveComponent* component, HCameraComponent* camera);

private:
	void UpdateComponent(HSceneComponent* component, float Delta);

	void RenderSceneViewFromCamera(FSceneView* view, HCameraComponent* camera);

	void BlitFromViewportToTarget(FViewPort* viewPort, NVRHI::TextureHandle target);
};