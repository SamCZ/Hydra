#pragma once

#include "Hydra/Framework/Object.h"

namespace Hydra
{
	class FWorld;

	class HYDRA_API AActor : public HObject
	{
	private:
		FWorld* _World;
	public:
		FWorld* GetWorld();
		void SetWorld(FWorld* world);
	};
}