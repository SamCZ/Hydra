#include "DrawState.h"

#include "Hydra/EngineContext.h"
#include "Hydra/Render/Material.h"
#include "Hydra/Render/VertexBuffer.h"
#include "Hydra/Framework/StaticMesh.h"

FDrawState::FDrawState()
{
	_State.renderState.depthStencilState.depthEnable = true;
	_State.renderState.rasterState.cullMode = NVRHI::RasterState::CULL_NONE;
	//_State.renderState.rasterState.fillMode = NVRHI::RasterState::FILL_LINE;
}

FDrawState::~FDrawState()
{
}

void FDrawState::SetClearFlags(bool clearColor, bool clearDepth, bool clearStencil)
{
	_State.renderState.clearColorTarget = clearColor;
	_State.renderState.clearDepthTarget = clearDepth;
	_State.renderState.clearStencilTarget = clearStencil;
}

void FDrawState::SetClearColor(const ColorRGBA & color)
{
	_State.renderState.clearColor = NVRHI::Color(color.r, color.g, color.b, color.a);
}

void FDrawState::SetViewPort(int width, int height)
{
	_State.renderState.viewportCount = 1;
	_State.renderState.viewports[0] = NVRHI::Viewport(width, height);
}

void FDrawState::SetViewPort(const Vector2i& size)
{
	SetViewPort(size.x, size.y);
}

void FDrawState::SetTargetCount(int count)
{
	_State.renderState.targetCount = count;
}

void FDrawState::SetTarget(int index, NVRHI::TextureHandle target)
{
	_State.renderState.targets[index] = target;
}

void FDrawState::SetTargets(NVRHI::TextureHandle* target, int count)
{
	SetTargetCount(count);

	for (uint8 i = 0; i < count; i++)
	{
		SetTarget(i, target[i]);
	}
}

void FDrawState::SetDepthTarget(NVRHI::TextureHandle target)
{
	_State.renderState.depthTarget = target;
}

void FDrawState::SetMaterial(MaterialInterface* materialInterface)
{
	materialInterface->ApplyParams(_State);

	_State.VS.shader = materialInterface->GetRawShader(NVRHI::ShaderType::SHADER_VERTEX);
	_State.HS.shader = materialInterface->GetRawShader(NVRHI::ShaderType::SHADER_HULL);
	_State.DS.shader = materialInterface->GetRawShader(NVRHI::ShaderType::SHADER_DOMAIN);
	_State.GS.shader = materialInterface->GetRawShader(NVRHI::ShaderType::SHADER_GEOMETRY);
	_State.PS.shader = materialInterface->GetRawShader(NVRHI::ShaderType::SHADER_PIXEL);
}

void FDrawState::SetInputLayout(NVRHI::InputLayoutHandle inputLayout)
{
	_State.inputLayout = inputLayout;
}

void FDrawState::SetIndexBuffer(NVRHI::BufferHandle buffer)
{
	_State.indexBufferFormat = NVRHI::Format::R32_UINT;
	_State.indexBuffer = buffer;
}

void FDrawState::SetVertexBuffer(NVRHI::BufferHandle buffer)
{
	if (_State.vertexBufferCount == 0)
	{
		_State.vertexBufferCount = 1;
	}

	_State.vertexBuffers[0].buffer = buffer;
	_State.vertexBuffers[0].slot = 0;
	_State.vertexBuffers[0].stride = sizeof(VertexBufferEntry);
}

void FDrawState::SetInstanceBuffer(NVRHI::BufferHandle buffer)
{
	_State.vertexBufferCount = 2;
	_State.vertexBuffers[1].buffer = buffer;
	_State.vertexBuffers[1].slot = 1;
	_State.vertexBuffers[1].stride = sizeof(Matrix4);
}


void FDrawState::Draw(NVRHI::IRendererInterface* renderInterface, int startVertex, int startIndex, int indexCount, int startInstaceIndex, int instanceCount)
{
	if (_State.vertexBufferCount == 0)
	{
		return;
	}

	NVRHI::DrawArguments args;

	args.startIndexLocation = startIndex;
	args.startVertexLocation = startVertex;
	args.instanceCount = instanceCount;
	args.startInstanceLocation = startInstaceIndex;
	args.vertexCount = indexCount;

	renderInterface->drawIndexed(_State, &args, 1);
}
