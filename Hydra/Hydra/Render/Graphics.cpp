#include "Hydra/Render/Graphics.h"

namespace Hydra
{
	void Graphics::SetShader(NVRHI::DrawCallState& state, ShaderPtr shader)
	{
		state.VS.shader = shader->GetShader(NVRHI::ShaderType::SHADER_VERTEX);
		state.HS.shader = shader->GetShader(NVRHI::ShaderType::SHADER_HULL);
		state.DS.shader = shader->GetShader(NVRHI::ShaderType::SHADER_DOMAIN);
		state.GS.shader = shader->GetShader(NVRHI::ShaderType::SHADER_GEOMETRY);
		state.PS.shader = shader->GetShader(NVRHI::ShaderType::SHADER_PIXEL);
	}
	void Graphics::SetClearFlags(NVRHI::DrawCallState& state, const ColorRGBA& color)
	{
		state.renderState.clearColor = NVRHI::Color(color.r, color.g, color.b, color.a);
		state.renderState.clearColorTarget = true;
		state.renderState.clearDepthTarget = true;
	}
}