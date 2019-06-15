#include "IndustryEmpire.h"

#include "Hydra/Framework/World.h"
#include "Hydra/Framework/GameModeBase.h"

#include "Game/FirstPersonCharacter.h"
#include "Game/CubeActor.h"

#include "GeneratedHeaders/GameClassDatabase.generated.h"

IndustryEmpire::IndustryEmpire()
{
	Game_InitializeClassDatabase();
}

void IndustryEmpire::SceneInit()
{
	World->SpawnActor<FirstPersonCharacter>("Player", Vector3(0, 0, 0), Vector3());

	World->SpawnActor<ACubeActor>("Cube", Vector3(), Vector3(), Vector3(0.01, 0.01, 0.01));
	//World->SpawnActor<ACubeActor>("Cube2", Vector3(0, 0, 1.5f), Vector3());

	World->OverrideGameMode<HGameModeBase>();
}
