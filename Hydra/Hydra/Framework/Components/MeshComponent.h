#pragma once

#include "Hydra/Framework/Components/PrimitiveComponent.h"

namespace Hydra
{
	class HYDRA_API HMeshComponent : public HPrimitiveComponent
	{
	public:
		virtual ~HMeshComponent() = 0;

		virtual List<class Material*> GetMaterials() const;
		virtual int32 GetMaterialIndex(String MaterialSlotName) const;
		virtual List<String> GetMaterialSlotNames() const;
		virtual bool IsMaterialSlotNameValid(String MaterialSlotName) const;

		virtual int32 GetNumMaterials() const override;
		virtual Material* GetMaterial(int32 ElementIndex) const override;
		virtual void SetMaterial(int32 ElementIndex, Material* Material) override;
		virtual void SetMaterialByName(String MaterialSlotName, class Material* Material) override;
	};
}