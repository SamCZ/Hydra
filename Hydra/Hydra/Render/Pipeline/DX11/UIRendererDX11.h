#pragma once

#include <d3d11.h>

#include "Hydra/Core/Container.h"
#include "Hydra/Render/UI/UIRenderer.h"

namespace Hydra
{
	class EngineContext;

	class UIRendererDX11 : public UIRenderer
	{
	private:
		EngineContext* _Context;
		ID3D11Device* _Device;
		Map<NVRHI::TextureHandle, int> _TexMap;
	public:
		UIRendererDX11(EngineContext* context);
	protected:
		virtual NVGcontext* CreateContext(int flags);
		virtual void DestroyContext(NVGcontext* context);
		virtual int GetHandleForTexture(NVGcontext* context, NVRHI::TextureHandle handle);
	};
}