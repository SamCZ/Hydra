#pragma once

#include "Hydra/Core/SmartPointer.h"
#include "Hydra/Core/String.h"
#include "Hydra/Core/Container.h"

#include "Hydra/Scene/Transformable.h"
#include "Hydra/Scene/Component.h"

namespace Hydra
{
	class Spatial : public Transformable
	{
	private:
		List<Spatial*> _Childs;
		List<Component*> _Components;
	public:
		String Name;
		Spatial* Parent;

		virtual ~Spatial();
		Spatial();
		Spatial(const String& name);

		void AddChild(Spatial* spatial);
		void RemoveChild(Spatial* spatial);
		List<Spatial*>& GetChilds();
		size_t GetChildCount();

		Spatial* GetChild(int index);

		void PrintHiearchy(int depth = 0) const;

		String GetHiearchy() const;

		template<class T> inline T* GetComponent()
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
			for (Component* cmp : _Components)
			{
				if (dynamic_cast<T*>(cmp))
				{
					return (T*)cmp;
				}
			}
			return nullptr;
		}

		template<class T> inline T* AddComponent()
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
			Component* component = GetComponent<T>();
			if (component == nullptr)
			{
				component = new T();
				component->Parent = this;
				_Components.push_back(component);
			}
			return (T*)component;
		}

		template<class T> inline T* RemoveComponent(bool autoDelete = true)
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
			for (int i = 0; i < _Components.size(); i++)
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
			}
			return nullptr;
		}

		virtual Matrix4 GetModelMatrix() override;
	};
}