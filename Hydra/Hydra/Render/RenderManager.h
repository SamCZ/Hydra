#pragma once

#include "Hydra/Core/Library.h"
#include "Hydra/Core/Common.h"

#include "Hydra/Render/View/ViewPort.h"

class EngineContext;

class HYDRA_API RenderManager
{
private:
	EngineContext* _Context;
public:
	RenderManager(EngineContext* context);
	~RenderManager();

	FViewPort* AddOrGetViewPort(const String& Name);
	void DeleteViewPort(FViewPort* ViewPort);
	void DeleteViewPort(const String& ViewPortName);

};