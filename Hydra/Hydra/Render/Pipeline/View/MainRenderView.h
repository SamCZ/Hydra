#pragma once

#include "Hydra/Render/Pipeline/DeviceManager.h"

namespace Hydra
{
	class HYDRA_API HydraEngine;

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
	};
}