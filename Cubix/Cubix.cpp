#include "Cubix.h"

#include "Hydra/Framework/World.h"
#include "Game/Generation/WorldGeneratorActor.h"

#include "Game/CubixGameMode.h"

#include "GeneratedHeaders/GameClassDatabase.generated.h"

Cubix::Cubix()
{
	Game_InitializeClassDatabase();
}

void Cubix::SceneInit()
{
	World->SpawnActor<WorldGeneratorActor>("WorldGenerator", Vector3(), Vector3());

	World->OverrideGameMode<CubixGameMode>();
}
