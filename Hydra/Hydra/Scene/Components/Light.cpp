#include "Hydra/Scene/Components/Light.h"

namespace Hydra
{
	Light::Light() : Type(LightType::Directional), Color(MakeRGB(250, 242, 218)), Range(1.0), SpotAngle(1.0), Intensity(1.0), IndirectMultiplier(1.0), ShadowType(ShadowType::Soft), CullMask(0), DepthBias(0.005f)
	{

	}
	void Light::Start()
	{

	}

	void Light::Update()
	{

	}
}