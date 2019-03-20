#include "Hydra/Render/RenderManager.h"
#include "Hydra/Scene/Spatial.h"
#include "Hydra/Scene/Components/Renderer.h"
#include "Hydra/Scene/Components/Light.h"

namespace Hydra
{
	List<SharedPtr<Renderer>>* YO = nullptr;
	List<SharedPtr<Light>>* YOO = nullptr;

	List<SharedPtr<Renderer>> RenderManager::GetRenderersForStage(const String & stage)
	{
		if (YO == nullptr)
		{
			YO = new List<SharedPtr<Renderer>>(MainScene->FindComponents<Renderer>());
		}

		return *YO;
	}

	List<SharedPtr<Light>> RenderManager::GetLights(const String & stage)
	{
		return MainScene->FindComponents<Light>();
	}
}