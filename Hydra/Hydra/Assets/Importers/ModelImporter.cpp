#include "ModelImporter.h"

#include "Hydra/Framework/StaticMesh.h"

bool ModelImporter::Import(Blob& dataBlob, const AssetImportOptions& options, HAsset*& out_Asset)
{
	if (options.IsA<ModelImportOptions>() == false)
	{
		LogError("ModelImporter::Import", "Wrong options data !");
		return false;
	}

	const ModelImportOptions* modelOptions = (&options)->SafeConstCast<ModelImportOptions>();

	int flags = aiProcessPreset_TargetRealtime_Quality |
		aiProcess_FindInstances |
		aiProcess_ValidateDataStructure |
		aiProcess_OptimizeMeshes /*| aiProcess_OptimizeGraph*/ | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFileFromMemory(dataBlob.GetData(), dataBlob.GetDataSize(), flags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LogError("ModelImporter::Import", importer.GetErrorString());
		return false;
	}

	Log(scene->mRootNode->mChildren[0]->mName.C_Str());

	//out_Asset = new HStaticMesh();

	return true;
}
