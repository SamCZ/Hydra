#pragma once

#include "Hydra/Framework/GameModeBase.h"
#include "CubixGameMode.generated.h"

#include "World/World.h"


HCLASS()
class CubixGameMode : public HGameModeBase
{
	HCLASS_GENERATED_BODY()
private:
	WorldPtr m_World;
public:
	CubixGameMode();
	virtual ~CubixGameMode();

	WorldPtr GetWorld();
};
