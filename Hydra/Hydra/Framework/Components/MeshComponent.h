#pragma once

#include "Hydra/Framework/Components/PrimitiveComponent.h"
#include "MeshComponent.generated.h"


HCLASS()
class HYDRA_API HMeshComponent : public HPrimitiveComponent
{
	HCLASS_GENERATED_BODY()
public:
	FORCEINLINE HMeshComponent() : HPrimitiveComponent() {}
	FORCEINLINE virtual ~HMeshComponent() {}

	/*virtual List<class Material*> GetMaterials() const;
	virtual int32 GetMaterialIndex(String MaterialSlotName) const;
	virtual List<String> GetMaterialSlotNames() const;
	virtual bool IsMaterialSlotNameValid(String MaterialSlotName) const;

	virtual int32 GetNumMaterials() const override;
	virtual Material* GetMaterial(int32 ElementIndex) const override;
	virtual void SetMaterial(int32 ElementIndex, Material* Material) override;
	virtual void SetMaterialByName(String MaterialSlotName, class Material* Material) override;*/
};
