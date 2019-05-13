#pragma once

#include "Hydra/Framework/Pawn.h"
#include "Character.generated.h"


HCLASS()
class HYDRA_API ACharacter : public APawn
{
	HCLASS_GENERATED_BODY()
public:
	ACharacter();
	virtual ~ACharacter();
};
