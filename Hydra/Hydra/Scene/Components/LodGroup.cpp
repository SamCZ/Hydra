#include "LodGroup.h"

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

		SelectLod(0);
	}

	void LodGroup::Update()
	{
		static int slod = 0;
		static float f = 0;

		f += 0.001f;

		if (f > 1.0)
		{
			f = 0;
			slod++;
		}


		if (slod >= 3) slod = 0;

		SelectLod(slod);
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
