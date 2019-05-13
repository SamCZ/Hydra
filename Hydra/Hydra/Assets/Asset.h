#pragma once

#include "Hydra/Framework/Object.h"
#include "Asset.generated.h"


HCLASS()
class HYDRA_API HAsset : public HObject
{
	HCLASS_GENERATED_BODY()
public:
	FORCEINLINE virtual ~HAsset() {}
};
