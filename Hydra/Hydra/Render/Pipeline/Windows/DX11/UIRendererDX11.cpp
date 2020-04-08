#include "UIRendererDX11.h"

//#define NANOVG_D3D11_IMPLEMENTATION
//#include "NanoVG/nanovg_d3d11.h"

#include "GFSDK_NVRHI_D3D11.h"
#include "Hydra/Render/Pipeline/Windows/DX11/DeviceManager11.h"

#include "Hydra/EngineContext.h"

UIRendererDX11::UIRendererDX11(EngineContext* context) : _Context(context), _Device((static_cast<DeviceManagerDX11*>(context->GetDeviceManager()))->GetDevice())
{

}

void UIRendererDX11::SetRenderTarget(NVRHI::TextureHandle tex)
{
	NVRHI::RendererInterfaceD3D11* renderInterface = (NVRHI::RendererInterfaceD3D11*)_Context->GetRenderInterface();
	ID3D11DeviceContext* m_ImmediateContext = nullptr;
	(static_cast<DeviceManagerDX11*>(_Context->GetDeviceManager()))->GetDevice()->GetImmediateContext(&m_ImmediateContext);

	ID3D11RenderTargetView* m_BackBufferRTV = renderInterface->getRTVForTexture(tex);

	const NVRHI::TextureDesc& desc = tex->GetDesc();

	D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (float)desc.width, (float)desc.height, 0.0f, 1.0f };

	static float clearColor[4]{ 0, 0, 0, 0 };

	m_ImmediateContext->ClearRenderTargetView(m_BackBufferRTV, clearColor);

	m_ImmediateContext->OMSetRenderTargets(1, &m_BackBufferRTV, nullptr);
	m_ImmediateContext->RSSetViewports(1, &viewport);
}

NVGcontext* UIRendererDX11::CreateContext(int flags)
{
	//return nvgCreateD3D11(_Device, flags | NVG_ANTIALIAS | NVG_STENCIL_STROKES);
	return nullptr;
}

void UIRendererDX11::DestroyContext(NVGcontext* context)
{
	//nvgDeleteD3D11(context);
}

int UIRendererDX11::GetHandleForTexture(NVGcontext* context, NVRHI::TextureHandle handle)
{
	/*NVGparams* params = nvgInternalParams(context);

	struct D3DNVGcontext* D3D = (struct D3DNVGcontext*)(params->userPtr);

	if (_TexMap.find(handle) != _TexMap.end())
	{
		return _TexMap[handle];
	}

	struct D3DNVGtexture* newTexStruct = D3Dnvg__allocTexture(D3D);

	NVRHI::TextureDesc desc = handle->GetDesc();

	NVRHI::RendererInterfaceD3D11* renderInterface = (NVRHI::RendererInterfaceD3D11*)_Context->GetRenderInterface();

	newTexStruct->width = desc.width;
	newTexStruct->height = desc.height;
	newTexStruct->type = NVG_TEXTURE_RGBA;
	newTexStruct->flags = NVG_IMAGE_NODELETE;
	newTexStruct->resourceView = renderInterface->getSRVForTexture(handle, DXGI_FORMAT_UNKNOWN, desc.mipLevels);

	int newId = newTexStruct->id;

	_TexMap[handle] = newId;

	return newId;*/
	return 0;
}