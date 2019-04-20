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
		aiProcess_OptimizeMeshes/* | aiProcess_OptimizeGraph*/ | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace/* | aiProcess_PreTransformVertices*/;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFileFromMemory(dataBlob.GetData(), dataBlob.GetDataSize(), flags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LogError("ModelImporter::Import", importer.GetErrorString());
		return false;
	}

	ProcessNode(scene, scene->mRootNode, "");

	//out_Asset = new HStaticMesh();

	return true;
}

void ModelImporter::ProcessNode(const aiScene* scene, aiNode* node, const String& node_hiearchy_name)
{

	for (uint8 i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		
		
	}

	if (node->mNumMeshes > 0)
	{
		Log(node_hiearchy_name + ": " + ToString(node->mNumMeshes));
	}

	for (uint8 i = 0; i < node->mNumChildren; i++)
	{
		String name = node_hiearchy_name;

		if (name.length() > 0)
		{
			name += "_";
		}

		name += node->mChildren[i]->mName.C_Str();

		ProcessNode(scene, node->mChildren[i], name);
	}
}
