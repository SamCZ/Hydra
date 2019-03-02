#include "LodGroup.h"
#include "Hydra/Scene/Components/Camera.h"

namespace Hydra
{
	void LodGroup::Start()
	{
		List<SpatialPtr> objectLods = Parent->FindAllApprox("L_LOD");

		for (int i = 0; i < 6; i++)
		{
			for (SpatialPtr obj : objectLods)
			{
				if ((int)obj->Name.substr(5)[0] - 48 == i)
				{
					_Lods.push_back(obj);
				}
			}
		}

		_LodMax = _Lods.size();

		SelectLod(0);
	}

	void LodGroup::Update()
	{
		const Vector3& objPos = Parent->Position;
		CameraPtr camera = Camera::MainCamera;

		if (camera)
		{
			const Vector3& cameraPos = camera->Parent->Position;
			float frustumSizeZ = camera->GetZFar() - camera->GetZNear();

			float distanceToObj = glm::abs(glm::distance(cameraPos, objPos));

			float disPercent = distanceToObj / frustumSizeZ;

			if (disPercent < 0.05)
			{
				SelectLod(0);
			}
			else
			{
				SelectLod(_LodMax);
			}
		}
	}

	void LodGroup::SelectLod(int lod)
	{
		for (int i = 0; i < _Lods.size(); i++)
		{
			if (lod == i)
			{
				_Lods[i]->Enabled = true;
			}
			else
			{
				_Lods[i]->Enabled = false;
			}
		}
	}
}
