#include "CubixGameMode.h"

CubixGameMode::CubixGameMode()
{
	m_World = MakeShared<World>();
}

CubixGameMode::~CubixGameMode()
{
}

WorldPtr CubixGameMode::GetWorld()
{
	return m_World;
}
