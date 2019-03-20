#pragma once

#include "Hydra/Scene/Spatial.h"

namespace Hydra
{
	class Material;
	class Renderer;
	class Mesh;

	class TerrainNode : public Spatial
	{
	private:
		SharedPtr<Renderer> _Renderer;
		Vector2i _Location;
		float _Size;
		int _Lod;

		float _LodRanges[8];
	public:
		TerrainNode(SharedPtr<Material> material, const Vector2i& position, float size, int lod);

		virtual void Start();
		virtual void Update();

	private:
		void SetChildsEnabled(bool enabled);
		void SetIsLeaf(bool leaf);

		static Mesh* CreatePatch(const Vector2i& pos, float size);
	};
}