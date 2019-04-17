#pragma once

#include "Hydra/Core/String.h"
#include "Hydra/Core/ColorRGBA.h"

namespace Hydra
{
	struct DeviceCreationParameters
	{
		int Width;
		int Height;
		int AutoPercentScale;
		bool AutoScale;
		bool FullScreen;
		bool Maximized;

		int RefreshRate;
		int SwapChainBufferCount;
		bool EnableDebugRuntime;

		String Title;
		ColorRGBA ClearColor;

		DeviceCreationParameters() :
			Width(1280),
			Height(720),
			AutoPercentScale(70),
			AutoScale(false),
			FullScreen(false),
			Maximized(false),
			RefreshRate(0),
			SwapChainBufferCount(1),
			EnableDebugRuntime(false),
			Title("Hydra Engine v0.0.5"),
			ClearColor(ColorRGBA::Black)
		{
		}
	};
}