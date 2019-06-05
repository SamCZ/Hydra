#pragma once

#include "Hydra/Core/Vector.h"
#include "Hydra/Core/ColorRGBA.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

class FViewPort;
class MaterialInterface;
class FStaticMesh;

class FDrawState
{
private:
	NVRHI::DrawCallState _State;
public:
	FDrawState();
	~FDrawState();

	void SetClearFlags(bool clearColor, bool clearDepth, bool clearStencil);
	void SetClearColor(const ColorRGBA& color);

	void SetViewPort(int width, int height);
	void SetViewPort(const Vector2i& size);

	void SetTargetCount(int count);
	void SetTarget(int index, NVRHI::TextureHandle target);
	void SetTargets(NVRHI::TextureHandle* target, int count);

	void SetMaterial(MaterialInterface* materialInterface);
	void SetInputLayout(NVRHI::InputLayoutHandle inputLayout);

	void SetIndexBuffer(NVRHI::BufferHandle buffer);
	void SetVertexBuffer(NVRHI::BufferHandle buffer);
	void SetInstanceBuffer(NVRHI::BufferHandle buffer);

	void Draw(NVRHI::IRendererInterface* renderInterface, int startIndex, int indexCount, int startInstaceIndex, int instanceCount);
};