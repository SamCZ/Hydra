#pragma once

#include "Hydra/Core/Library.h"
#include "Hydra/Render/Pipeline/DeviceCreationParameters.h"
#include "Hydra/EngineContext.h"

namespace Hydra
{
	class FWorld;

	class HYDRA_API HydraEngine
	{
	protected:
		EngineContext* Context;
		FWorld* World;

	public:
		void Start();
		
	public:
		virtual void PrepareForEngineStart(DeviceCreationParameters& params);
		virtual void InitializeAssetManager(AssetManager* assetManager);
		virtual void SceneInit() = 0;

		FWorld* GetWorld() const;
		EngineContext* GetContext() const;
	};
}