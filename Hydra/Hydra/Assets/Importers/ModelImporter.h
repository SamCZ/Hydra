#pragma once

#include "Hydra/Assets/IAssetImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class ModelImportOptions : public AssetImportOptions
{
	HCLASS_BODY(ModelImportOptions)
public:
	bool CombineMeshes;
};

class ModelImporter : public IAssetImporter
{
public:
	bool Import(Blob& dataBlob, const AssetImportOptions& options, HAsset*& out_Asset);

private:

	void ProcessNode(const aiScene* scene, aiNode* node, const String& node_hiearchy_name);
};