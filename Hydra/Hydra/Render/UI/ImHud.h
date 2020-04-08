#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Core/Vector.h"

class EngineContext;

namespace ImHud
{
	void Init(EngineContext* context);

	bool Checkbox(const String& id, const Vector2& pos, bool* checked);
}