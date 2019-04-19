#pragma once

#include "Object.h"

struct FStaticMaterial
{
	class MaterialInterface* Material;
	String MaterialSlotName;

	FStaticMaterial() : Material(nullptr), MaterialSlotName(String_None) {}
	FStaticMaterial(class MaterialInterface* Material, String materialSlotName) : Material(Material), MaterialSlotName(materialSlotName) {}

	bool operator==(const FStaticMaterial& left) { return Material == left.Material && MaterialSlotName == left.MaterialSlotName; }
};

class HStaticMesh : public HObject
{
public:

	class FStaticMeshRenderData* RenderData;

	List<FStaticMaterial> StaticMaterials;
};