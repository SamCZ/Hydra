#include "Hydra/Scene/Component.h"

namespace Hydra
{
	Component::Component() : Enabled(true)
	{
	}

	int Component::CollideWith(const Collidable& c, CollisionResults & results)
	{
		return 0;
	}
}