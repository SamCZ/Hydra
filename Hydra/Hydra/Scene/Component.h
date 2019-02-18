#pragma once

namespace Hydra
{
	class Spatial;

	class Component
	{
	public:
		Spatial* Parent;
		bool Enabled;

		Component();

		virtual void Start() = 0;
		virtual void Update() = 0;
	};

	
}