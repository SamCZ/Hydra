#pragma once
 
#include "Hydra/Core/Common.h"
#include "Hydra/Core/SmartPointer.h"
#include "Hydra/Core/String.h"
#include "Hydra/Core/Container.h"

#include "Hydra/Scene/Transformable.h"
#include "Hydra/Scene/Component.h"

namespace Hydra
{
	class Spatial;

	DEFINE_PTR(Spatial)

	class Spatial : public Transformable
	{
	private:
		List<SpatialPtr> _Childs;
		List<ComponentPtr> _Components;
	public:
		String Name;
		Spatial* Parent;

		virtual ~Spatial();
		Spatial();
		Spatial(const String& name);

		void AddChild(SpatialPtr spatial);
		void RemoveChild(SpatialPtr spatial);
		List<SpatialPtr>& GetChilds();
		size_t GetChildCount();

		SpatialPtr GetChild(int index);

		void Update();

		void PrintHiearchy(int depth = 0) const;

		String GetHiearchy() const;

		template<class T> FORCEINLINE SharedPtr<T> GetComponent()
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
			for (ComponentPtr cmp : _Components)
			{
				if (cmp)
				{
					auto castedCmp = std::dynamic_pointer_cast<T, Component>(cmp);

					if (castedCmp)
					{
						return castedCmp;
					}
				}
			}

			return SharedPtr<T>();
		}

		template<class T> FORCEINLINE SharedPtr<T> AddComponent()
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
			SharedPtr<T> component = GetComponent<T>();
			if (component == nullptr)
			{
				component = New(T);
				component->Parent = this;
				_Components.push_back(component);
			}

			return component;
		}

		template<class T> FORCEINLINE void RemoveComponent()
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

			for (int i = 0; i < _Components.size(); i++)
			{
				ComponentPtr component = _Components[i];

				if (std::dynamic_pointer_cast<T, Component>(component))
				{
					_Components.erase(_Components.begin() + i);
				}
			}

			/*for (int i = 0; i < _Components.size(); i++)
			{
				Component* component = _Components[i];
				if (dynamic_cast<T*>(component))
				{
					_Components.erase(_Components.begin() + i);
					component->Parent = nullptr;

					if (autoDelete)
					{
						delete component;
						return nullptr;
					}

					return (T*)component;
				}
			}*/
		}

		virtual Matrix4 GetModelMatrix() override;
	};
}