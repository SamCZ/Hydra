#include "IndustryEmpire.h"

void IndustryEmpire::SceneInit()
{
	HWorld* world = WorldContext->CreateWorld("Main");

	world->SpawnPawn<BasicPlayerPawn>("Hráč", Vector3(), Vector3());

	world->OverrideGameMode<MyGameMode>();
}
