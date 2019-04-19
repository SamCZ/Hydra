#pragma once

#include "Hydra/Assets/IAssetImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class ModelImportOptions : public AssetImportOptions
{
	HCLASS_BODY(ModelImportOptions)
};

class ModelImporter : public IAssetImporter
{
public:
	bool Import(Blob& dataBlob, const AssetImportOptions& options, HAsset*& out_Asset);
};