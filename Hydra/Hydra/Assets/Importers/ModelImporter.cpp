#include "ModelImporter.h"

#include "FreeImage/FreeImage.h"

#include "Hydra/Core/File.h"
#include "Hydra/Framework/StaticMesh.h"
#include "Hydra/Framework/StaticMeshResources.h"

static std::vector<std::string> TextureTypeEnumToString = {
				"aiTextureType_NONE", "aiTextureType_DIFFUSE", "aiTextureType_SPECULAR",
				"aiTextureType_AMBIENT", "aiTextureType_EMISSIVE", "aiTextureType_HEIGHT",
				"aiTextureType_NORMALS", "aiTextureType_SHININESS", "aiTextureType_OPACITY",
				"aiTextureType_DISPLACEMENT", "aiTextureType_LIGHTMAP", "aiTextureType_REFLECTION",
				"aiTextureType_UNKNOWN", "_aiTextureType_Force32Bit" };

static Map<uint8, String> PrimitiveTypeToString{
	{(uint8)aiPrimitiveType::aiPrimitiveType_POINT, "aiPrimitiveType_POINT"},
	{(uint8)aiPrimitiveType::aiPrimitiveType_LINE, "aiPrimitiveType_LINE"},
	{(uint8)aiPrimitiveType::aiPrimitiveType_TRIANGLE, "aiPrimitiveType_TRIANGLE"},
	{(uint8)aiPrimitiveType::aiPrimitiveType_POLYGON, "aiPrimitiveType_POLYGON"},
	{(uint8)aiPrimitiveType::_aiPrimitiveType_Force32Bit, "_aiPrimitiveType_Force32Bit"}
};

bool ModelImporter::Import(Blob& dataBlob, const AssetImportOptions& options, List<HAsset*>& out_Assets)
{
	if (options.IsA<ModelImportOptions>() == false)
	{
		LogError("ModelImporter::Import", "Wrong options data !");
		return false;
	}

	const ModelImportOptions* modelOptions = (&options)->SafeConstCast<ModelImportOptions>();

	int flags = aiProcessPreset_TargetRealtime_Quality |
		/*aiProcess_FindInstances |*/+
		aiProcess_ValidateDataStructure |
		aiProcess_OptimizeMeshes/* | aiProcess_OptimizeGraph*/ | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace/* | aiProcess_PreTransformVertices*/;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFileFromMemory(dataBlob.GetData(), dataBlob.GetDataSize(), flags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LogError("ModelImporter::Import", importer.GetErrorString());
		return false;
	}

	Map<String, aiTexture*> embeddedTextures;

	for (uint32 i = 0; i < scene->mNumTextures; i++)
	{
		aiTexture* tex = scene->mTextures[i];

		File file = tex->mFilename.C_Str();

		embeddedTextures[file.GetName()] = tex;
	}

	List<HStaticMesh*> staticMeshes;

	int materialSlotIndex = 0;

	ProcessNode(scene, scene->mRootNode, embeddedTextures, String_None, *modelOptions, staticMeshes, materialSlotIndex);

	for (HStaticMesh* mesh : staticMeshes)
	{
		out_Assets.push_back(mesh);
	}

	return true;
}

void ModelImporter::ProcessNode(const aiScene* scene, aiNode* node, Map<String, aiTexture*> embeddedTextures, const String& node_hiearchy_name, const ModelImportOptions& options, List<HStaticMesh*>& staticMeshes, int& materialSlotIndex)
{
	List<aiMesh*> triangleMeshes;

	// Setup triangle meshes
	for (uint32 i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		if (mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE)
		{
			triangleMeshes.push_back(mesh);
		}
	}

	// Create static mesh instance
	HStaticMesh* staticMesh = nullptr;
	FStaticMeshLODResources* meshResources = nullptr;

	if (options.CombineMeshes)
	{
		if (staticMeshes.size() == 1)
		{
			staticMesh = staticMeshes[0];
			meshResources = &staticMesh->RenderData->LODResources[0];
		}
		else
		{
			staticMesh = new HStaticMesh();
			staticMesh->RenderData->LODResources.push_back({});
			staticMeshes.push_back(staticMesh);

			meshResources = &staticMesh->RenderData->LODResources[0];
		}
	}
	else if (triangleMeshes.size() > 0)
	{
		materialSlotIndex = 0;
		staticMesh = new HStaticMesh();
		staticMesh->RenderData->LODResources.push_back({});
		staticMeshes.push_back(staticMesh);

		meshResources = &staticMesh->RenderData->LODResources[0];
	}

	// Load mesh
	for (uint32 i = 0; i < triangleMeshes.size(); i++)
	{
		aiMesh* mesh = triangleMeshes[i];

		FStaticMeshSection section = {};
		section.MaterialIndex = -1;

		ProcessMesh(mesh, node, staticMesh, *meshResources, section, i);

		// Load mesh materials
		if (mesh->mMaterialIndex >= 0)
		{
			section.MaterialIndex = materialSlotIndex;
			materialSlotIndex++;

			FStaticMaterial staticMaterial = {};
			staticMaterial.Material = nullptr;
			staticMaterial.MaterialSlotName = String("Material_") + ToString(section.MaterialIndex);
			staticMesh->StaticMaterials.push_back(staticMaterial);

			aiMaterial* sourceMaterial = scene->mMaterials[mesh->mMaterialIndex];

			// Load material textures
			for (uint8 texType = 0; texType < (uint32)aiTextureType::aiTextureType_UNKNOWN; texType++)
			{
				int textureCount = sourceMaterial->GetTextureCount((aiTextureType)texType);

				if (textureCount > 0)
				{
					aiString texturePathAiStr;

					if (sourceMaterial->GetTexture((aiTextureType)texType, 0, &texturePathAiStr) == aiReturn_SUCCESS)
					{
						File filePath = File(texturePathAiStr.C_Str());
						String fileName = filePath.GetName();

						auto iter = embeddedTextures.find(fileName);

						if (iter != embeddedTextures.end())
						{
							aiTexture* tex = iter->second;

							unsigned char* aiData = (unsigned char*)tex->pcData;



							/*FIMEMORY* hmem = FreeImage_OpenMemory(aiData, tex->mWidth);
							FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(hmem, 0);
							Log(filePath.GetCleanName() + ".png");

							FIBITMAP *check = FreeImage_LoadFromMemory(fif, hmem, 0);
							FreeImage_Save(FIF_PNG, check, (filePath.GetCleanName() + ".png").c_str(), 0);
							FreeImage_Unload(check);

							FreeImage_CloseMemory(hmem);*/
						}
					}
				}
			}
		}

		meshResources->Sections.emplace_back(section);
	}

	if (node->mNumMeshes > 0)
	{
		//Log(node_hiearchy_name + ": " + ToString(node->mNumMeshes));
	}

	// Iterate over children
	for (uint32 i = 0; i < node->mNumChildren; i++)
	{
		String name = node_hiearchy_name;

		if (name.length() > 0)
		{
			name += "_";
		}

		name += node->mChildren[i]->mName.C_Str();

		ProcessNode(scene, node->mChildren[i], embeddedTextures, name, options, staticMeshes, materialSlotIndex);
	}
}

void ModelImporter::ProcessMesh(aiMesh* mesh, aiNode* node, HStaticMesh* staticMesh, FStaticMeshLODResources& meshResources, FStaticMeshSection& meshSection, int index)
{
	aiMatrix4x4 transform = node->mTransformation;

	meshSection.MinVertexIndex = meshResources.VertexData.size();

	for (uint32 vertIdx = 0u; vertIdx < mesh->mNumVertices; vertIdx++)
	{
		VertexBufferEntry vertexEntry = {};

		aiVector3D vert = transform * mesh->mVertices[vertIdx];

		vertexEntry.Position.x = vert.x;
		vertexEntry.Position.y = vert.y;
		vertexEntry.Position.z = vert.z;

		if (mesh->HasNormals())
		{
			aiVector3D norm = transform * mesh->mNormals[vertIdx];

			vertexEntry.Normal.x = norm.x;
			vertexEntry.Normal.y = norm.y;
			vertexEntry.Normal.z = norm.z;
		}

		if (mesh->HasTangentsAndBitangents())
		{
			aiVector3D tan = transform * mesh->mTangents[vertIdx];
			aiVector3D bit = transform * mesh->mBitangents[vertIdx];

			vertexEntry.Tangent.x = tan.x;
			vertexEntry.Tangent.y = tan.y;
			vertexEntry.Tangent.z = tan.z;

			vertexEntry.BiTangent.x = bit.x;
			vertexEntry.BiTangent.y = bit.y;
			vertexEntry.BiTangent.z = bit.z;
		}

		if (mesh->mTextureCoords[0])
		{
			aiVector3D tex = mesh->mTextureCoords[0][vertIdx];

			vertexEntry.TexCoord.x = tex.x;
			vertexEntry.TexCoord.y = tex.y;
		}

		meshResources.VertexData.emplace_back(vertexEntry);
	}

	meshSection.MaxVertexIndex = meshResources.VertexData.size();

	int lastIndex = meshResources.LastIndex;

	meshSection.FirstIndex = lastIndex;

	for (std::uint32_t faceIdx = 0u; faceIdx < mesh->mNumFaces; faceIdx++)
	{
		for (uint8 id = 0; id < 3; id++)
		{
			uint32 index = mesh->mFaces[faceIdx].mIndices[id];

			meshResources.Indices.push_back(index);

			meshResources.LastIndex++;
		}
	}

	meshSection.NumTriangles = mesh->mNumFaces * 3;

	return;
}
