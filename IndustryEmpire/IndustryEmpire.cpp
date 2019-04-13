#include "IndustryEmpire.h"

#include "Hydra/Framework/World.h"
#include "Hydra/Framework/GameModeBase.h"

void IndustryEmpire::SceneInit()
{
	FWorld* world = nullptr;

	world->SpawnActor<AActor>("Player", Vector3(), Vector3());

	world->OverrideGameMode<HGameModeBase>();
}
