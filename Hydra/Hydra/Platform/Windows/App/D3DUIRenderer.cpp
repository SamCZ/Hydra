#include "D3DUIRenderer.h"

#define NANOVG_D3D11_IMPLEMENTATION
#include "NanoVG/nanovg_d3d11.h"

D3DUIRenderer::D3DUIRenderer(NVRHI::RendererInterfaceD3D11 * renderInterface, ID3D11Device * dxDevice) : RenderInterface(renderInterface), GD3DDevice(dxDevice)
{
}

D3DUIRenderer::~D3DUIRenderer()
{
}

NVGcontext* D3DUIRenderer::CreateContext(int flags)
{
	return nvgCreateD3D11(GD3DDevice, flags | NVG_ANTIALIAS | NVG_STENCIL_STROKES);
}

void D3DUIRenderer::DestroyContext(NVGcontext * context)
{
	nvgDeleteD3D11(context);
}

int D3DUIRenderer::GetHandleForTexture(NVGcontext * context, NVRHI::TextureHandle handle)
{
	NVGparams* params = nvgInternalParams(context);

	struct D3DNVGcontext* D3D = (struct D3DNVGcontext*)(params->userPtr);

	if (_TexMap.find(handle) != _TexMap.end())
	{
		return _TexMap[handle];
	}

	struct D3DNVGtexture* newTexStruct = D3Dnvg__allocTexture(D3D);

	NVRHI::TextureDesc desc = handle->GetDesc();

	newTexStruct->width = desc.width;
	newTexStruct->height = desc.height;
	newTexStruct->type = NVG_TEXTURE_RGBA;
	newTexStruct->flags = NVG_IMAGE_NODELETE;
	newTexStruct->resourceView = RenderInterface->getSRVForTexture(handle, DXGI_FORMAT_UNKNOWN, desc.mipLevels);

	int newId = newTexStruct->id;

	_TexMap[handle] = newId;

	return newId;
}
