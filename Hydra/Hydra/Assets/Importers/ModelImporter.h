#pragma once

#include "Hydra/Assets/IAssetImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class HStaticMesh;
struct FStaticMeshLODResources;
struct FStaticMeshSection;

HCLASS()
class ModelImportOptions : public AssetImportOptions
{
	HCLASS_GENERATED_BODY()
public:
	bool CombineMeshes;
};

class ModelImporter : public IAssetImporter
{
public:
	bool Import(Blob& dataBlob, const AssetImportOptions& options, List<HAsset*>& out_Assets);

private:

	void ProcessNode(const aiScene* scene, aiNode* node, Map<String, aiTexture*> embeddedTextures, const String& node_hiearchy_name, const ModelImportOptions& options, List<HStaticMesh*>& staticMeshes, int& materialSlotIndex);
	void ProcessMesh(aiMesh* mesh, aiNode* node, HStaticMesh* staticMesh, FStaticMeshLODResources& meshResources, FStaticMeshSection& meshSection, int index);
};