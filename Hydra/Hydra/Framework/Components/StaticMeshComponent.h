#pragma once

#include "MeshComponent.h"
#include "Hydra/Framework/StaticMesh.h"

HCLASS()
class HYDRA_API HStaticMeshComponent final : public HMeshComponent
{
	HCLASS_GENERATED_BODY()
public:
	HStaticMesh* StaticMesh;

public:
	HStaticMeshComponent();
	~HStaticMeshComponent();
};