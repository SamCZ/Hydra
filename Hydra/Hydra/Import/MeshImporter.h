#pragma once

#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Hydra/Scene/Spatial.h"
#include "Hydra/Core/File.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/String.h"

namespace Hydra
{
	struct MeshImportOptions
	{
		bool CombineMeshes;
	};

	class Mesh;

	class Meshimporter
	{
	public:

		static SpatialPtr Import(const File& file, MeshImportOptions& importoptions);

	private:
		static void ProcessNode(const aiScene* aScene, aiNode* aNode, SpatialPtr rootScene, SpatialPtr scene, String material, const File& rootFolder);

		static void ProcessAnimations(const aiScene* scene, SpatialPtr m);

		static Mesh* ProcessMesh(aiMesh *mesh, const aiScene *scene, SpatialPtr rootAnimScene);
	};
}