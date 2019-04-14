#pragma once

#include "Hydra/Framework/Components/SceneComponent.h"

namespace Hydra
{
	class Material;

	class HPrimitiveComponent : public HSceneComponent
	{
	public:
		float LDMaxDrawDistance;

		uint8 ReceivesDecals : 1;
		uint8 OwnerNoSee : 1;
		uint8 OnlyOwnerSee : 1;
		uint8 CastShadow : 1;
		uint8 CastDynamicShadow : 1;
		uint8 CastStaticShadow : 1;
		uint8 SelfShadowOnly : 1;
		uint8 CastFarShadow : 1;
		uint8 CastShadowAsTwoSided : 1;

		virtual Matrix4 GetRenderMatrix() const;

		virtual int32 GetNumMaterials() const;
		virtual Material* GetMaterial(int32 ElementIndex) const;
		virtual void SetMaterial(int32 ElementIndex, Material* Material);
		virtual void SetMaterialByName(String MaterialSlotName, Material* Material);
	};
}