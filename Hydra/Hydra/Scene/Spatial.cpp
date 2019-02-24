#include "Hydra/Scene/Spatial.h"
#include <iostream>
#include "Hydra/Scene/Components/Camera.h"

namespace Hydra
{
	Spatial::~Spatial()
	{
	}

	Spatial::Spatial() : Spatial("Unknown")
	{
	}

	Spatial::Spatial(const String & name) : Name(name), Parent(nullptr)
	{
		Position = Vector3(0, 0, 0);
		Rotation = Vector3(0, 0, 0);
		Scale = Vector3(1.0, 1.0, 1.0);
	}

	void Spatial::AddChild(SpatialPtr spatial)
	{
		if (spatial != nullptr)
		{
			spatial->Parent = this;
			_Childs.push_back(spatial);
		}
	}

	void Spatial::RemoveChild(SpatialPtr spatial)
	{
		if (spatial != nullptr)
		{
			spatial->Parent = nullptr;
			List_Remove(_Childs, spatial);
		}
	}

	List<SpatialPtr>& Spatial::GetChilds()
	{
		return _Childs;
	}

	size_t Spatial::GetChildCount()
	{
		return _Childs.size();
	}

	SpatialPtr Spatial::GetChild(int index)
	{
		if (index < 0 || index >= _Childs.size())
		{
			return nullptr;
		}

		return _Childs[index];
	}

	Matrix4 Spatial::GetModelMatrix()
	{
		if (GetComponent<Camera>())
		{
			return Matrix4();
		}

		if (Parent != nullptr)
		{
			return Parent->GetModelMatrix() * Transformable::GetModelMatrix();
		}
		else
		{
			return Transformable::GetModelMatrix();
		}
	}

	void Spatial::PrintHiearchy(int depth) const
	{
		for (int i = 0; i < depth; i++)
		{
			std::cout << "-";
		}

		std::cout << " " << Name << std::endl;

		for (SpatialPtr child : _Childs)
		{
			child->PrintHiearchy(depth + 1);
		}
	}

	String Spatial::GetHiearchy() const
	{
		String hiearchy;
		const Spatial* spatial = this;

		while (spatial != nullptr)
		{
			hiearchy = spatial->Name + "/" + hiearchy;
			spatial = spatial->Parent;
		}

		return hiearchy;
	}
}