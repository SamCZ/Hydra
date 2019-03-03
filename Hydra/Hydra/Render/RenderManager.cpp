#include "Hydra/Render/RenderManager.h"
#include "Hydra/Scene/Spatial.h"
#include "Hydra/Scene/Components/Renderer.h"

namespace Hydra
{
	List<SharedPtr<Renderer>>* YO = nullptr;

	List<SharedPtr<Renderer>> RenderManager::GetRenderersForStage(const String & stage)
	{
		if (YO == nullptr)
		{
			YO = new List<SharedPtr<Renderer>>(MainScene->FindComponents<Renderer>());
		}

		return *YO;
	}
}