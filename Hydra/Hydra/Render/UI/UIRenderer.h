#pragma once

#include "Hydra/Core/String.h"
#include "Hydra/Core/ColorRGBA.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

struct NVGcontext;

namespace Hydra
{
	enum class Align
	{
		Baseline,
		Bottom,
		Center,
		Left,
		Right,
		Top
	};

	class UIRenderer
	{
	private:
		NVGcontext* _Context;
		float _TextBounds[4];
	public:
		UIRenderer();

		void Create();
		void Destroy();

		void Begin();
		void End();

		void DrawImage(NVRHI::TextureHandle tex, float x, float y, float width, float height, float borderRadius, float alpha = 1.0f);

		void DrawString(const String& text, float x, float y, float fontSize, const ColorRGBA& color, const Align& alignX = Align::Left, const Align& alignY = Align::Top);

		float GetTextSize(const String& text, float fontSize, float* textHeight);

		int BeginClip(float x, float y, float width, float height);
		void EndClip(int id);
		void ClearClipping();

		void DrawRect(float x, float y, float w, float h, ColorRGBA color, bool isStroke = false, float strokeSize = 0.0f, float topLeft = 0.0f, float topRight = 0.0f, float bottomRight = 0.0f, float bottomLeft = 0.0f);

		void DrawOval(float x, float y, float w, float h, ColorRGBA color);

	protected:
		virtual NVGcontext* CreateContext(int flags) = 0;
		virtual void DestroyContext(NVGcontext* context) = 0;

		virtual int GetHandleForTexture(NVGcontext* context, NVRHI::TextureHandle handle) = 0;
	};
}