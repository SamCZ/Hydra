#include "Hydra/Render/UI/UIRenderer.h"

#include "NanoVG/nanovg.h"

#include "Hydra/Scene/Components/Camera.h"

namespace Hydra
{
	UIRenderer::UIRenderer() : _Context(nullptr)
	{

	}

	void UIRenderer::Create()
	{
		if (_Context == nullptr)
		{
			_Context = CreateContext(0);

			nvgCreateFont(_Context, "Font", "Assets/Fonts/arial.ttf");
			//nvgCreateFont(_Context, "FontAwesome", "Assets/Fonts/fontawesome-webfont.ttf");
		}
	}

	void UIRenderer::Destroy()
	{
		if (_Context != nullptr)
		{
			DestroyContext(_Context);
		}
	}

	void UIRenderer::Begin()
	{
		nvgBeginFrame(_Context, Camera::MainCamera->GetWidth(), Camera::MainCamera->GetHeight(), 1.0f);
	}

	void UIRenderer::End()
	{
		nvgEndFrame(_Context);
	}

	void UIRenderer::DrawImage(NVRHI::TextureHandle tex, float x, float y, float width, float height, float borderRadius, float alpha)
	{
		int handle = GetHandleForTexture(_Context, tex);
		NVGpaint paint = nvgImagePattern(_Context, x, y, width, height, 0.0f, handle, 1.0f);

		nvgBeginPath(_Context);
		if (borderRadius > 0)
		{
			nvgRoundedRect(_Context, x, y, width, height, borderRadius);
		}
		else
		{
			nvgRect(_Context, x, y, width, height);
		}
		nvgFillPaint(_Context, paint);
		nvgFill(_Context);
	}

	void UIRenderer::DrawString(const String& text, float x, float y, float fontSize, const ColorRGBA& color, const Align& alignX, const Align& alignY)
	{
		nvgFontSize(_Context, fontSize);
		nvgFontFace(_Context, "Font");

		int flags = 0;
		switch (alignX)
		{
		case Align::Left:
			flags |= NVG_ALIGN_LEFT;
			break;
		case Align::Right:
			flags |= NVG_ALIGN_RIGHT;
			break;
		case Align::Center:
			flags |= NVG_ALIGN_CENTER;
			break;
		}

		switch (alignY)
		{
		case Align::Top:
			flags |= NVG_ALIGN_TOP;
			break;
		case Align::Bottom:
			flags |= NVG_ALIGN_BOTTOM;
			break;
		case Align::Center:
			flags |= NVG_ALIGN_MIDDLE;
			break;
		}
		nvgFontBlur(_Context, 0);
		nvgGlobalAlpha(_Context, 1.0f);
		nvgTextAlign(_Context, flags);
		nvgFillColor(_Context, nvgRGBAf(color.r, color.g, color.b, color.a));
		nvgText(_Context, x, y, text.c_str(), NULL);
	}

	float UIRenderer::GetTextSize(const String& text, float fontSize, float* textHeight)
	{
		nvgFontSize(_Context, fontSize);
		nvgFontFace(_Context, "Font");
		nvgTextAlign(_Context, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

		float w = nvgTextBounds(_Context, 0.0f, 0.0f, text.c_str(), NULL, _TextBounds);
		if (textHeight)
		{
			*textHeight = (_TextBounds[3] - _TextBounds[1]);
		}
		return w;
	}

	int UIRenderer::BeginClip(float x, float y, float width, float height)
	{
		nvgSave(_Context);
		nvgIntersectScissor(_Context, x, y, width, height);
		return 0;
	}

	void UIRenderer::EndClip(int id)
	{
		nvgRestore(_Context);
	}

	void UIRenderer::ClearClipping()
	{

	}

	void UIRenderer::DrawRect(float x, float y, float w, float h, ColorRGBA color, bool isStroke, float strokeSize, float topLeft, float topRight, float bottomRight, float bottomLeft)
	{

		NVGpaint shadowPaint = nvgBoxGradient(_Context, x, y + 2, w, h, topLeft * 2, 10, nvgRGBA(0, 0, 0, 128), nvgRGBA(0, 0, 0, 0));
		nvgBeginPath(_Context);
		nvgRect(_Context, x - 10, y - 10, w + 20, h + 30);
		nvgRoundedRect(_Context, x, y, w, h, topLeft);
		nvgPathWinding(_Context, NVG_HOLE);
		nvgFillPaint(_Context, shadowPaint);
		nvgFill(_Context);

		nvgBeginPath(_Context);
		if (topLeft == topRight && bottomRight == bottomLeft && topLeft == bottomRight && topRight == bottomLeft)
		{
			nvgRoundedRect(_Context, x, y, w, h, topLeft);
		}
		else
		{
			nvgRoundedRectVarying(_Context, x, y, w, h, topLeft, topRight, bottomRight, bottomLeft);
		}
		NVGcolor c = nvgRGBAf(color.r, color.g, color.b, 1.0f);
		if (isStroke)
		{
			nvgStrokeWidth(_Context, strokeSize);
			nvgStrokeColor(_Context, c);
			nvgStroke(_Context);
		}
		else
		{
			nvgFillColor(_Context, c);
			nvgFill(_Context);
		}
	}

	void UIRenderer::DrawOval(float x, float y, float w, float h, ColorRGBA color)
	{
		nvgBeginPath(_Context);

		nvgCircle(_Context, x, y, w);

		NVGcolor c = nvgRGBAf(color.r, color.g, color.b, 1.0f);

		/*nvgFillColor(_Context, c);
		nvgFill(_Context);*/

		nvgStrokeWidth(_Context, 1.0f);
		nvgStrokeColor(_Context, c);
		nvgStroke(_Context);
	}
}