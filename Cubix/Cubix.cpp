#include "Cubix.h"

#include "Hydra/Framework/World.h"
#include "Game/Generation/WorldGeneratorActor.h"

#include "Game/CubixGameMode.h"
#include "Game/FirstPersonCharacter.h"
#include "Game/CubeActor.h"

#include "GeneratedHeaders/GameClassDatabase.generated.h"

Cubix::Cubix()
{
	Game_InitializeClassDatabase();
}

void Cubix::SceneInit()
{
	World->SpawnActor<FirstPersonCharacter>("Player", Vector3(0, 0, 0), Vector3());
	World->SpawnActor<ACubeActor>("SkyBox", Vector3(), Vector3(), Vector3(100, 100, 100));

	World->SpawnActor<WorldGeneratorActor>("WorldGenerator", Vector3(), Vector3());

	World->OverrideGameMode<CubixGameMode>();
}
