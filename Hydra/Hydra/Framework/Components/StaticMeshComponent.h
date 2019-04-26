#pragma once

#include "MeshComponent.h"
#include "Hydra/Framework/StaticMesh.h"

class HYDRA_API HStaticMeshComponent final : public HMeshComponent
{
	HCLASS_BODY(HStaticMeshComponent)
public:
	HStaticMesh* StaticMesh;

public:
	HStaticMeshComponent();
	~HStaticMeshComponent();
};