#include "Cubix.h"

#include "Hydra/Framework/World.h"
#include "Game/Generation/WorldGeneratorActor.h"

#include "Game/CubixGameMode.h"
#include "Game/FirstPersonCharacter.h"

#include "GeneratedHeaders/GameClassDatabase.generated.h"

Cubix::Cubix()
{
	Game_InitializeClassDatabase();
}

void Cubix::SceneInit()
{
	World->SpawnActor<FirstPersonCharacter>("Player", Vector3(0, 0, 0), Vector3());

	World->SpawnActor<WorldGeneratorActor>("WorldGenerator", Vector3(), Vector3());

	World->OverrideGameMode<CubixGameMode>();
}
