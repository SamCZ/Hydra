#include "Hydra/Render/UI/UIRenderer.h"

#include "NanoVG/nanovg.h"
#include "Hydra/Core/File.h"

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

void UIRenderer::Begin(const Vector2& screenSize)
{
	nvgBeginFrame(_Context, screenSize.x, screenSize.y, 1.0f);
}

void UIRenderer::End()
{
	nvgEndFrame(_Context);
}

void UIRenderer::DrawImage(NVRHI::TextureHandle tex, float x, float y, float width, float height, float borderRadius, float alpha, float angle)
{
	bool flip = !tex->GetDesc().isRenderTarget;

	int handle = GetHandleForTexture(_Context, tex);
	NVGpaint paint;

	if (flip)
	{
		paint = nvgImagePattern(_Context, x, y + height, width, -height, angle, handle, 1.0f);
	}
	else
	{
		paint = nvgImagePattern(_Context, x, y, width, height, angle, handle, 1.0f);
	}

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
	/* Shadow
	NVGpaint shadowPaint = nvgBoxGradient(_Context, x, y + 2, w, h, topLeft * 2, 10, nvgRGBA(0, 0, 0, 128), nvgRGBA(0, 0, 0, 0));
	nvgBeginPath(_Context);
	nvgRect(_Context, x - 10, y - 10, w + 20, h + 30);
	nvgRoundedRect(_Context, x, y, w, h, topLeft);
	nvgPathWinding(_Context, NVG_HOLE);
	nvgFillPaint(_Context, shadowPaint);
	nvgFill(_Context);*/

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

NVGcolor FromRGBA(const ColorRGBA& color)
{
	return nvgRGBAf(color.r, color.g, color.b, color.a);
}

void UIRenderer::DrawLine(float x0, float y0, float x1, float y1, float strength, const ColorRGBA & startColor, const ColorRGBA & endColor)
{
	NVGpaint paint = nvgLinearGradient(_Context, x0, y0, x1, y1, FromRGBA(startColor), FromRGBA(endColor));

	nvgBeginPath(_Context);
	nvgMoveTo(_Context, x0, y0);
	nvgLineTo(_Context, x1, y1);

	nvgStrokeWidth(_Context, strength);
	nvgStrokePaint(_Context, paint);
	nvgStroke(_Context);
}

void UIRenderer::DrawLine(float x0, float y0, float x1, float y1, float strength, const ColorRGBA & color)
{
	DrawLine(x0, y0, x1, y1, strength, color, color);
}

void UIRenderer::RB_RenderLine(float x1, float y1, float x2, float y2, float strokeWidth, const ColorRGBA & color)
{
	nvgBeginPath(_Context);
	nvgMoveTo(_Context, x1, y1);
	nvgLineTo(_Context, x2, y2);
	nvgStrokeColor(_Context, FromRGBA(color));
	nvgStrokeWidth(_Context, strokeWidth);
	nvgStroke(_Context);
}

void UIRenderer::RB_RenderGradient(float x, float y, float width, float height, const ColorRGBA & leftColor, const ColorRGBA & rightColor, float radius)
{
	const NVGpaint paint = nvgLinearGradient(_Context, x, y, x + width, y, FromRGBA(leftColor), FromRGBA(rightColor));
	nvgBeginPath(_Context);
	nvgFillPaint(_Context, paint);
	nvgRoundedRect(_Context, x, y, width, height, radius);
	nvgClosePath(_Context);
	nvgFill(_Context);
}

void UIRenderer::RB_RenderBlock(const String & title, float x, float y, float width, float height, bool isSelected)
{
	DrawRect(x, y, width, height, MakeRGBA(37, 37, 38, 210)); //Background
	
	RB_RenderGradient(x, y, width, 25, MakeRGB(20, 100, 200), MakeRGB(4, 40, 80), 2.0f);

	DrawString(title, x + width / 2.0f, y + 2, 20, MakeRGB(181, 188, 188), Align::Center); //Title

	if (isSelected)
	{
		DrawRect(x, y, width, height, MakeRGB(82, 158, 207), true, 1.5f, 2.0f); //Outline
	}
	else
	{
		DrawRect(x, y, width, height, MakeRGB(100, 100, 100), true, 1.5f, 2.0f); //Outline
	}
}

void UIRenderer::RB_RenderSpline(float x1, float y1, float x2, float y2, int count, float strokeWidth)
{
	NVGcolor color;
	color.a = 1.0f;
	color.r = 1.0f;
	color.g = 1.0f;
	color.b = 1.0f;

	static NVGcolor colors[4] {

	};

	float spsz = sqrtf((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) * 0.5f;
	float spacing = 3.3f;
	float tbsw = spacing;

	if (x2 < x1)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	if (y2 < y1)
	{
		tbsw *= -1;
	}

	for (int i = 0; i < count; i++)
	{
		if (count > 1)
		{
			
		}

		nvgBeginPath(_Context);
		nvgMoveTo(_Context, x1, y1 + (spacing * i));
		nvgBezierTo(_Context, x1 + spsz - (tbsw * i), y1 + (spacing * i), x2 - spsz - (tbsw * i), y2 + (spacing * i), x2, y2 + (spacing * i));
		nvgStrokeColor(_Context, color);
		nvgStrokeWidth(_Context, strokeWidth);
		nvgStroke(_Context);
	}
}
