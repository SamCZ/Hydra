#include "ImHud.h"

#include "Hydra/EngineContext.h"

namespace ImHud
{
	static EngineContext* Context;
	static UIRenderer* UIR;
	static InputManager* INM;

	bool operator>(const Vector2& left, const Vector2& right)
	{
		return left.x > right.x && left.y > right.y;
	}

	bool operator<(const Vector2& left, const Vector2& right)
	{
		return left.x < right.x && left.y < right.y;
	}

	void Init(EngineContext* context)
	{
		Context = context;
		UIR = context->GetUIRenderer();
		INM = context->GetInputManager();
	}

	bool CheckFocusAndClick(const String& id, bool& out_focused, const Vector2& pos, const Vector2& size)
	{
		Vector2 cpos = INM->GetCursorPos();

		out_focused = cpos > pos && cpos < (pos + size);

		return false;
	}

	bool Checkbox(const String& id, const Vector2& pos, bool* checked)
	{
		ColorRGBA backgroundColor = MakeRGB(174, 174, 174);
		ColorRGBA checkColor = MakeRGB(102, 102, 102);

		static ColorRGBA focusColor = MakeRGBA(40, 40, 40, 0);

		float styleSize = 16;
		float hSize = styleSize * 0.5f;

		float checkThickness = 2.0f;
		float radius = 2.0f;

		Vector2 newPos = Vector2(pos.x - hSize, pos.y - hSize);
		Vector2 size = Vector2(styleSize, styleSize);

		bool focused = false;
		bool clicked = false;

		if (clicked = CheckFocusAndClick(id, focused, newPos, size))
		{
			*checked = !checked;
		}

		if (focused && !clicked)
		{
			backgroundColor += focusColor;
			checkColor += focusColor;
		}

		UIR->DrawRect(newPos.x, newPos.y, size.x, size.y, backgroundColor, false, 0.0f, radius, radius, radius, radius);
		UIR->DrawCheck(pos.x, pos.y, checkColor, checkThickness);

		return clicked;
	}
}