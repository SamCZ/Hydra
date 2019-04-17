#include "IndustryEmpire.h"

#include "Hydra/Framework/World.h"
#include "Hydra/Framework/GameModeBase.h"

#include "Game/CubeActor.h"

void IndustryEmpire::SceneInit()
{
	World->SpawnActor<ACubeActor>("Cube", Vector3(), Vector3());
	World->SpawnActor<ACubeActor>("Cube2", Vector3(0, 0, 1.5f), Vector3());

	World->OverrideGameMode<HGameModeBase>();

}
