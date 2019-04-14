#pragma once

#include "Hydra/Core/Library.h"
#include "Hydra/Render/Pipeline/DeviceCreationParameters.h"
#include "Hydra/EngineContext.h"

namespace Hydra
{
	class HYDRA_API HydraEngine
	{
	protected:
		EngineContext* Context;

	public:
		void Start();
		
	public:
		virtual void PrepareForEngineStart(DeviceCreationParameters& params);
		virtual void SceneInit() = 0;
	};
}