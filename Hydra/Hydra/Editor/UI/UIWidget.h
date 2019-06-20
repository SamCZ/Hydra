#pragma once

#include "Hydra/Core/Common.h"

class UIWidget
{
public:

	virtual ~UIWidget() = 0;

	int32 OnPaint();
};