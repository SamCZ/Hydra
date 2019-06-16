#pragma once

#include "Hydra/Core/String.h"
#include "Hydra/Core/ColorRGBA.h"
#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

struct NVGcontext;

enum class Align
{
	Baseline,
	Bottom,
	Center,
	Left,
	Right,
	Top
};

class HYDRA_API UIRenderer
{
private:
	NVGcontext* _Context;
	float _TextBounds[4];
public:
	UIRenderer();

	void Create();
	void Destroy();

	void Begin(const Vector2& screenSize);
	void End();
	virtual void SetRenderTarget(NVRHI::TextureHandle tex) = 0;

	void DrawImage(NVRHI::TextureHandle tex, float x, float y, float width, float height, float borderRadius = 0.0f, float alpha = 1.0f, float angle = 0.0f);

	void DrawString(const String& text, float x, float y, float fontSize, const ColorRGBA& color, const Align& alignX = Align::Left, const Align& alignY = Align::Top);

	float GetTextSize(const String& text, float fontSize, float* textHeight);

	int BeginClip(float x, float y, float width, float height);
	void EndClip(int id);
	void ClearClipping();

	void DrawRect(float x, float y, float w, float h, ColorRGBA color, bool isStroke = false, float strokeSize = 0.0f, float topLeft = 0.0f, float topRight = 0.0f, float bottomRight = 0.0f, float bottomLeft = 0.0f);
	void DrawOval(float x, float y, float w, float h, ColorRGBA color);
	void DrawLine(float x0, float y0, float x1, float y1, float strength, const ColorRGBA& startColor, const ColorRGBA& endColor);
	void DrawLine(float x0, float y0, float x1, float y1, float strength, const ColorRGBA& color);

	void RB_RenderLine(float x1, float y1, float x2, float y2, float strokeWidth, const ColorRGBA& color);
	void RB_RenderGradient(float x, float y, float width, float height, const ColorRGBA& leftColor, const ColorRGBA& rightColor, float radius = 0.0f);
	void RB_RenderBlock(const String& title, float x, float y, float width, float height, bool isSelected);
	void RB_RenderSpline(float x1, float y1, float x2, float y2, int count, float strokeWidth = 1.5f);

protected:
	virtual NVGcontext* CreateContext(int flags) = 0;
	virtual void DestroyContext(NVGcontext* context) = 0;

	virtual int GetHandleForTexture(NVGcontext* context, NVRHI::TextureHandle handle) = 0;
};