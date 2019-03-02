#include "Hydra/Render/RenderManager.h"
#include "Hydra/Scene/Spatial.h"
#include "Hydra/Scene/Components/Renderer.h"

namespace Hydra
{
	List<SharedPtr<Renderer>> RenderManager::GetRenderersForStage(const String & stage)
	{
		return MainScene->FindComponents<Renderer>();
	}
}