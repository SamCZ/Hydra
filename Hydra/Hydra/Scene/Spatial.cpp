#include "Hydra/Scene/Spatial.h"
#include <iostream>
#include "Hydra/Scene/Components/Camera.h"

namespace Hydra
{
	Spatial::~Spatial()
	{
	}

	Spatial::Spatial()
	{
	}

	Spatial::Spatial(const String & name) : Name(name)
	{

	}

	void Spatial::AddChild(Spatial* spatial)
	{
		if (spatial != nullptr)
		{
			spatial->Parent = this;
			_Childs.push_back(spatial);
		}
	}

	void Spatial::RemoveChild(Spatial* spatial)
	{
		if (spatial != nullptr)
		{
			spatial->Parent = nullptr;
			List_Remove(_Childs, spatial);
		}
	}

	List<Spatial*>& Spatial::GetChilds()
	{
		return _Childs;
	}

	size_t Spatial::GetChildCount()
	{
		return _Childs.size();
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

		for (Spatial* child : _Childs)
		{
			child->PrintHiearchy(depth + 1);
		}
	}
}