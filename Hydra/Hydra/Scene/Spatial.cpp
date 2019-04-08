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

	Spatial::Spatial(const String & name) : Name(name), Parent(nullptr), _Enabled(true), _Static(false), _StaticMatrixGenrated(false)
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

	void Spatial::Start()
	{
		for (ComponentPtr cmp : _Components)
		{
			cmp->Start();
		}

		for (SpatialPtr child : _Childs)
		{
			child->Start();
		}
	}

	void Spatial::Update()
	{
		for (ComponentPtr cmp : _Components)
		{
			if (cmp->Enabled)
			{
				cmp->Update();
			}
		}

		for (SpatialPtr child : _Childs)
		{
			if (child->IsEnabled())
			{
				child->Update();
			}
		}
	}

	int Spatial::CollideWith(const Collidable& c, CollisionResults & results)
	{
		int count = 0;

		for (ComponentPtr cmp : _Components)
		{
			if (cmp->Enabled)
			{
				count += cmp->CollideWith(c, results);
			}
		}

		for (SpatialPtr child : _Childs)
		{
			if (child->IsEnabled())
			{
				count += child->CollideWith(c, results);
			}
		}

		return count;
	}

	Matrix4 Spatial::GetModelMatrix()
	{
		if (Parent)
		{
			return Parent->GetModelMatrix() * Transformable::GetModelMatrix(); //TODO: this is slow !
		}
		else
		{
			return Transformable::GetModelMatrix();
		}
	}

	void Spatial::SetMainCamera(SharedPtr<Component> camera)
	{
		if (!Camera::MainCamera)
		{
			Camera::MainCamera = std::dynamic_pointer_cast<Camera, Component>(camera);
		}

		Camera::AllCameras.push_back(std::dynamic_pointer_cast<Camera, Component>(camera));
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

	String Spatial::GetHiearchy()
	{
		String hiearchy;
		Spatial* spatial = this;

		while (spatial != nullptr)
		{
			hiearchy = spatial->Name + "/" + hiearchy;
			spatial = spatial->Parent;
		}

		return hiearchy;
	}

	SpatialPtr Spatial::Find(const String & name)
	{
		for (SpatialPtr child : GetChilds())
		{
			if (child->Name == name)
			{
				return child;
			}
		}

		for (SpatialPtr child : GetChilds())
		{
			SpatialPtr finded = child->Find(name);
			if (finded != nullptr)
			{
				return finded;
			}
		}

		return nullptr;
	}

	SpatialPtr Spatial::Find(long fileID)
	{
		for (SpatialPtr child : GetChilds())
		{
			if (child->FileID == fileID)
			{
				return child;
			}
		}

		for (SpatialPtr child : GetChilds())
		{
			SpatialPtr finded = child->Find(fileID);
			if (finded != nullptr)
			{
				return finded;
			}
		}

		return nullptr;
	}

	SpatialPtr Spatial::FindApprox(const String & name)
	{
		for (SpatialPtr child : GetChilds())
		{
			if (child->Name == name || StartsWith(child->Name, name))
			{
				return child;
			}
		}

		for (SpatialPtr child : GetChilds())
		{
			SpatialPtr finded = child->FindApprox(name);
			if (finded != nullptr)
			{
				return finded;
			}
		}

		return nullptr;
	}

	List<SpatialPtr> Spatial::FindAllApprox(const String & name)
	{
		List<SpatialPtr> list;

		for (SpatialPtr child : GetChilds())
		{
			if (child->Name == name || StartsWith(child->Name, name))
			{
				list.push_back(child);
			}
		}

		for (SpatialPtr child : GetChilds())
		{
			auto childFindings = child->FindAllApprox(name);

			list.insert(list.end(), childFindings.begin(), childFindings.end());
		}

		return list;
	}

	void Spatial::SetEnabled(bool enabled)
	{
		_Enabled = enabled;
	}

	bool Spatial::IsEnabled()
	{
		if (Parent)
		{
			if (!Parent->IsEnabled())
			{
				return false;
			}

			return _Enabled;
		}
		return _Enabled;
	}

	void Spatial::SetStatic(bool s)
	{
		_Static = s;

		GetStaticModelMatrix();

		for (SpatialPtr child : _Childs)
		{
			child->SetStatic(s);
		}
	}

	bool Spatial::IsStatic()
	{
		return _Static;
	}

	Matrix4& Spatial::GetStaticModelMatrix()
	{
		if (!_StaticMatrixGenrated)
		{
			_StaticMatrixGenrated = true;

			_StaticModelMatrix = GetModelMatrix();
		}

		return _StaticModelMatrix;
	}

	Vector3 Spatial::GetDirection()
	{
		Matrix4 mat = Transformable::GetModelMatrix();

		return Transformable::GetForward(mat);
	}
}