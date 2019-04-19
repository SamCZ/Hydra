#pragma once

#include "Hydra/Core/Library.h"
#include "Hydra/Render/Pipeline/DeviceCreationParameters.h"
#include "Hydra/EngineContext.h"

class FWorld;

class HYDRA_API HydraEngine
{
protected:
	EngineContext* Context;
	FWorld* World;

public:
	~HydraEngine();

	void Start();
	void OnDestroy();

public:
	virtual void PrepareForEngineStart(DeviceCreationParameters& params);
	virtual void InitializeAssetManager(AssetManager* assetManager);
	virtual void SceneInit() = 0;

	FWorld* GetWorld() const;
	EngineContext* GetContext() const;
};