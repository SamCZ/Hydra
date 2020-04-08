#pragma once

#include "Hydra/Core/Container.h"
#include "Hydra/Render/UI/UIRenderer.h"

#include "Hydra/Render/Pipeline/Windows/DX11/GFSDK_NVRHI_D3D11.h"

#include <d3d11.h>

class D3DUIRenderer : public UIRenderer
{
private:
	Map<NVRHI::TextureHandle, int> _TexMap;

	ID3D11Device* GD3DDevice;
	NVRHI::RendererInterfaceD3D11* RenderInterface;
public:
	D3DUIRenderer(NVRHI::RendererInterfaceD3D11* renderInterface, ID3D11Device* dxDevice);
	virtual ~D3DUIRenderer();
protected:
	virtual NVGcontext* CreateContext(int flags);
	virtual void DestroyContext(NVGcontext* context);
	virtual int GetHandleForTexture(NVGcontext* context, NVRHI::TextureHandle handle);

	virtual void SetRenderTarget(NVRHI::TextureHandle tex) { }
};