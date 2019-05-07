#pragma once

#include "Hydra/Assets/Asset.h"
#include "StaticMesh.generated.h"


struct FStaticMaterial
{
	class MaterialInterface* Material;
	String MaterialSlotName;

	FStaticMaterial() : Material(nullptr), MaterialSlotName(String_None) {}
	FStaticMaterial(class MaterialInterface* Material, String materialSlotName) : Material(Material), MaterialSlotName(materialSlotName) {}

	bool operator==(const FStaticMaterial& left) { return Material == left.Material && MaterialSlotName == left.MaterialSlotName; }
};

HCLASS()
class HStaticMesh : public HAsset
{
	HCLASS_GENERATED_BODY()
public:

	class FStaticMeshRenderData* RenderData;

	List<FStaticMaterial> StaticMaterials;

public:
	HStaticMesh();
	~HStaticMesh();
};
