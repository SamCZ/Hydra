#pragma once

#include "MeshComponent.h"
#include "Hydra/Framework/StaticMesh.h"
#include "StaticMeshComponent.generated.h"


HCLASS()
class HYDRA_API HStaticMeshComponent final : public HMeshComponent
{
	HCLASS_GENERATED_BODY()
public:
	HStaticMesh* StaticMesh;

public:
	HStaticMeshComponent();
	virtual ~HStaticMeshComponent();
};
