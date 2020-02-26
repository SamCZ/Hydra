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
	EWorld->OverrideGameMode<CubixGameMode>();

	EWorld->SpawnActor<FirstPersonCharacter>("Player", Vector3(0, 0, 0), Vector3());
	EWorld->SpawnActor<ACubeActor>("SkyBox", Vector3(), Vector3(), Vector3(100, 100, 100));

	EWorld->SpawnActor<WorldGeneratorActor>("WorldGenerator", Vector3(), Vector3());
}
