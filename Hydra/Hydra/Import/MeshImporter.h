#pragma once

#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Hydra/Core/File.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/String.h"

namespace Hydra
{
	struct MeshImportOptions
	{
		bool CombineMeshes;
	};

	class Spatial;
	class Mesh;

	class Meshimporter
	{
	public:

		static Spatial* Import(const File& file, MeshImportOptions& importoptions);

	private:
		static void ProcessNode(const aiScene* aScene, aiNode* aNode, Spatial* rootScene, Spatial* scene, String material);

		static void ProcessAnimations(const aiScene* scene, Spatial* m);

		static Mesh* ProcessMesh(aiMesh *mesh, const aiScene *scene, Spatial* rootAnimScene);
	};
}