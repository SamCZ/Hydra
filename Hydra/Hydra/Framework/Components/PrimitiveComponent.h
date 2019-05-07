#pragma once

#include "Hydra/Framework/Components/SceneComponent.h"
#include "PrimitiveComponent.generated.h"


class Material;

HCLASS()
class HYDRA_API HPrimitiveComponent : public HSceneComponent
{
	HCLASS_GENERATED_BODY()
public:
	float LDMaxDrawDistance;

	uint8 Registered : 1;

	uint8 ReceivesDecals : 1;
	uint8 OwnerNoSee : 1;
	uint8 OnlyOwnerSee : 1;
	uint8 CastShadow : 1;
	uint8 CastDynamicShadow : 1;
	uint8 CastStaticShadow : 1;
	uint8 SelfShadowOnly : 1;
	uint8 CastFarShadow : 1;
	uint8 CastShadowAsTwoSided : 1;
public:
	HPrimitiveComponent();

	/*virtual Matrix4 GetRenderMatrix() const;

	virtual int32 GetNumMaterials() const;
	virtual Material* GetMaterial(int32 ElementIndex) const;
	virtual void SetMaterial(int32 ElementIndex, Material* Material);
	virtual void SetMaterialByName(String MaterialSlotName, Material* Material);

	virtual void OnRegister();
	virtual void OnUnregister();

	void RegisterComponent();
	void UnregisterComponent();*/

	FORCEINLINE bool IsRegistered() const { return Registered; }
};
