#include "Hydra/Render/Pipeline/DX11/UIRendererDX11.h"

#define NANOVG_D3D11_IMPLEMENTATION
#include "NanoVG/nanovg_d3d11.h"

#include "Hydra/Render/Pipeline/GFSDK_NVRHI_D3D11.h"

#include "Hydra/EngineContext.h"

namespace Hydra
{
	UIRendererDX11::UIRendererDX11(EngineContext* context) : _Context(context), _Device(context->GetDeviceManager()->GetDevice())
	{
		
	}

	NVGcontext* UIRendererDX11::CreateContext(int flags)
	{
		return nvgCreateD3D11(_Device, flags | NVG_ANTIALIAS | NVG_STENCIL_STROKES);
	}

	void UIRendererDX11::DestroyContext(NVGcontext* context)
	{
		nvgDeleteD3D11(context);
	}

	int UIRendererDX11::GetHandleForTexture(NVGcontext* context, NVRHI::TextureHandle handle)
	{
		NVGparams* params = nvgInternalParams(context);

		struct D3DNVGcontext* D3D = (struct D3DNVGcontext*)(params->userPtr);

		if (_TexMap.find(handle) != _TexMap.end())
		{
			return _TexMap[handle];
		}

		struct D3DNVGtexture* newTexStruct = D3Dnvg__allocTexture(D3D);

		NVRHI::TextureDesc desc = handle->GetDesc();

		NVRHI::RendererInterfaceD3D11* renderInterface = (NVRHI::RendererInterfaceD3D11*)_Context->GetRenderInterface().get();

		newTexStruct->width = desc.width;
		newTexStruct->height = desc.height;
		newTexStruct->type = NVG_TEXTURE_RGBA;
		newTexStruct->flags = NVG_IMAGE_NODELETE;
		newTexStruct->resourceView = renderInterface->getSRVForTexture(handle, DXGI_FORMAT_UNKNOWN, desc.mipLevels);

		int newId = newTexStruct->id;

		_TexMap[handle] = newId;

		return newId;
	}

}