#pragma once

#include "Hydra/Core/Library.h"
#include "Hydra/Core/Common.h"

#include "Hydra/Render/View/ViewPort.h"

class HYDRA_API RenderManager
{
public:

	FViewPort* AddOrGetViewPort(const String& Name);
	void DeleteViewPort(FViewPort* ViewPort);
	void DeleteViewPort(const String& ViewPortName);

};