#pragma once

#include "Hydra/Core/Library.h"

namespace Hydra
{
	struct EngineStartParams
	{
		int Width;
		int Height;
		int AutoPercentScale;
		bool AutoScale;
		bool FullScreen;
		bool Maximized;

		EngineStartParams() : Width(1280), Height(720), AutoPercentScale(70), AutoScale(false), FullScreen(false), Maximized(false) { }
	};

	class HYDRA_API HydraEngine
	{
	protected:
		
	public:
		virtual void PrepareForEngineStart(EngineStartParams& params);

		virtual void SceneInit() = 0;
	};
}