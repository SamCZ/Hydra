#pragma once

#include "Hydra/Core/ColorRGBA.h"
#include "Hydra/Scene/Component.h"

namespace Hydra
{
	struct LightType
	{
		enum Enum
		{
			Spot, Directional, Point, Area
		};
	};

	struct ShadowType
	{
		enum Enum
		{
			None,
			Hard,
			Soft
		};
	};

	class Light : public Component
	{
	private:

	public:
		LightType::Enum Type;

		ColorRGBA Color;

		float Range;
		float SpotAngle;

		int AreaShape; //TODO: Area shape primitive

		float Intensity;
		float IndirectMultiplier;

		float DepthBias;

		ShadowType::Enum ShadowType;

		int CullMask;

		Light();

		virtual void Start();
		virtual void Update();
	};

	DEFINE_PTR(Light)
}