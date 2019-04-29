#pragma once

#include "Hydra/Framework/Object.h"
#include "Hydra/Framework/Components/SceneComponent.h"

class FWorld;
class EngineContext;
class HGameModeBase;

class UIRenderer;

class HYDRA_API AActor : public HObject
{
	HCLASS_BODY(AActor)
public:
	EngineContext* Engine;
	FWorld* World;

	uint8 IsIndestructible : 1;
	uint8 IsActive : 1;
	uint8 IsEditorOnly : 1;

	List<HSceneComponent*> Components;

	HSceneComponent* RootComponent;

	int Layer; // Added only for purpose sort Hud layer for LudumDare
public:
	AActor();

	virtual void InitializeComponents();

	template<class T>
	FORCEINLINE T* AddComponent(const String& name)
	{
		T* componentRaw = new T();

		HSceneComponent* component = reinterpret_cast<HSceneComponent*>(componentRaw);

		component->Name = name;

		InitilizeComponent(component);
		Components.push_back(component);

		if (RootComponent)
		{
			component->AttachToComponent(RootComponent);
		}

		return componentRaw;
	}

	void DestroyComponent(HSceneComponent*& component);

	virtual void BeginPlay();
	virtual void BeginDestroy();
	virtual void Tick(float DeltaTime);

	virtual void SetActive(bool newActive);
	virtual void ToggleActive();

	virtual void Destroy();

	Vector3& GetLocation();
	Vector3& GetRotation();
	Vector3& GetScale();

	void SetLocation(const Vector3& location);
	void SetRotation(const Vector3& rotation);
	void SetScale(const Vector3& scale);

	void SetLocation(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetScale(float x, float y, float z);

	void AddLocation(const Vector3& location);
	void AddRotation(const Vector3& rotation);
	void AddScale(const Vector3& scale);

	void AddLocation(float x, float y, float z);
	void AddRotation(float x, float y, float z);
	void AddScale(float x, float y, float z);

	/*Vector3 GetForwardVector() const;
	Vector3 GetUpVector() const;
	Vector3 GetRightVector() const;*/

	// Added only for purpose ludum dare, remove in future
	virtual void OnHud(UIRenderer* renderer)
	{

	}

	HGameModeBase* GetGameMode();

private:
	void InitilizeComponent(HSceneComponent* component);
};