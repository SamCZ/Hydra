#include "Hydra/Import/MeshImporter.h"

#include <iostream>
#include "Hydra/Core/Log.h"
#include "Hydra/Scene/Spatial.h"
#include "Hydra/Render/Mesh.h"
#include "Hydra/Scene/Components/Renderer.h"

namespace Hydra
{
	Spatial* Meshimporter::Import(const File& file, MeshImportOptions& importoptions)
	{
		std::cout << "Loading model: " << file << std::endl;

		Assimp::Importer importer;

		const aiScene *scene = importer.ReadFile(file.GetPath(), aiProcessPreset_TargetRealtime_Quality |
			aiProcess_FindInstances |
			aiProcess_ValidateDataStructure |
			aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace); //aiProcess_PreTransformVertices

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return nullptr;
		}

		Spatial* node = new Spatial(file.GetName());
		//node->setFileSource(file);

		ProcessAnimations(scene, node);
		ProcessNode(scene, scene->mRootNode, node, node, "");

		Log("Meshimporter::Import(" + file.GetPath() + ")", "Loaded.");

		return node;
	}

	glm::vec3 aiVecToGlm(const aiVector3D& vec)
	{
		return glm::vec3(vec.x, vec.y, vec.z);
	}

	void Meshimporter::ProcessNode(const aiScene* aScene, aiNode* aNode, Spatial* rootScene, Spatial* scene, String material)
	{
		Spatial* childScene = new Spatial(std::string(aNode->mName.C_Str()));
		scene->AddChild(childScene);

		aiVector3D position;
		aiVector3D rotation;
		aiVector3D scale;

		aiMatrix4x4 transform = aNode->mTransformation;
		transform.Decompose(scale, rotation, position);

		childScene->Position = aiVecToGlm(position);
		childScene->Rotation = glm::vec3(glm::degrees(rotation.x), glm::degrees(rotation.y), glm::degrees(rotation.z));
		childScene->Scale = aiVecToGlm(scale);

		for (unsigned int i = 0; i < aNode->mNumMeshes; i++)
		{
			aiMesh* sourceMesh = aScene->mMeshes[aNode->mMeshes[i]];

			Mesh* mesh = ProcessMesh(sourceMesh, aScene, rootScene);
			if (mesh->PrimitiveType != NVRHI::PrimitiveType::TRIANGLE_LIST)
			{
				delete mesh;
				continue;
			}

			Spatial* obj = new Spatial("Mesh #" + ToString(i + 1));
			Renderer* renderer = obj->AddComponent<Renderer>();
			renderer->SetMesh(mesh);

			if (sourceMesh->mMaterialIndex >= 0)
			{
				aiMaterial* souceMaterial = aScene->mMaterials[sourceMesh->mMaterialIndex];

				//TODO: Materials
			}

			childScene->AddChild(obj);
		}

		for (unsigned int i = 0; i < aNode->mNumChildren; i++)
		{
			ProcessNode(aScene, aNode->mChildren[i], rootScene, childScene, material);
		}
	}

	void Meshimporter::ProcessAnimations(const aiScene * scene, Spatial * m)
	{
		//TODO: Animations
		/*if (scene->HasAnimations())
		{
			Animator* animator = new Animator();
			for (int x = 0; x < scene->mNumAnimations; x++)
			{
				std::string AnimName = scene->mAnimations[x]->mName.data;
				if (AnimName.length() == 0)
				{
					AnimName = "Unknown";
				}

				Animation anim;
				anim.name = AnimName;
				anim.duration = scene->mAnimations[x]->mDuration;
				anim.ticksPerSecond = scene->mAnimations[x]->mTicksPerSecond;

				//scene->mAnimations[0]->mMorphMeshChannels[0]->

				std::cout << "Animation: " << AnimName << " - duration: " << anim.duration << " - tps: " << anim.ticksPerSecond << std::endl;

				for (int y = 0; y < scene->mAnimations[x]->mNumChannels; y++)
				{
					Animation::Channel channel;
					channel.name = scene->mAnimations[x]->mChannels[y]->mNodeName.data;
					if (channel.name.length() == 0)
					{
						channel.name = "Unknown";
					}

					std::cout << "Animation channel: " << channel.name << std::endl;

					for (int z = 0; z < scene->mAnimations[x]->mChannels[y]->mNumPositionKeys; z++)
					{
						channel.mPositionKeys.push_back(scene->mAnimations[x]->mChannels[y]->mPositionKeys[z]);
					}
					for (int z = 0; z < scene->mAnimations[x]->mChannels[y]->mNumRotationKeys; z++)
					{
						channel.mRotationKeys.push_back(scene->mAnimations[x]->mChannels[y]->mRotationKeys[z]);
					}

					for (int z = 0; z < scene->mAnimations[x]->mChannels[y]->mNumScalingKeys; z++)
					{
						channel.mScalingKeys.push_back(scene->mAnimations[x]->mChannels[y]->mScalingKeys[z]);
					}

					anim.channels.push_back(channel);
				}

				animator->CurrentAnim = 0;

				for (int z = 0; z < MAX_BONES; z++)
				{
					anim.boneTrans.push_back(glm::mat4(1.0f));
				}

				animator->Animations.push_back(anim);

			}
			m->setAnimator(animator);
			animator->Animations[animator->CurrentAnim].root.name = "rootBoneTreeNode";
		}*/
	}

	Mesh* Meshimporter::ProcessMesh(aiMesh * mesh, const aiScene * scene, Spatial * rootAnimScene)
	{
		Mesh* nMesh = new Mesh();

		switch (mesh->mPrimitiveTypes)
		{
		case aiPrimitiveType_POINT:
			nMesh->PrimitiveType = NVRHI::PrimitiveType::POINT_LIST;
			break;
		case aiPrimitiveType_LINE:
			nMesh->PrimitiveType = NVRHI::PrimitiveType::LINE_LIST;
			break;
		case aiPrimitiveType_TRIANGLE:
			nMesh->PrimitiveType = NVRHI::PrimitiveType::TRIANGLE_LIST;
			break;
		case aiPrimitiveType_POLYGON:
			nMesh->PrimitiveType = NVRHI::PrimitiveType::PATCH_1_CONTROL_POINT;
			break;
		default:
			break;
		}

#if DEBUG_ASSIMP
		std::cout << "Number of UV channels: " << mesh->GetNumUVChannels() << std::endl;
		std::cout << "Vertex color channel count: " << mesh->GetNumColorChannels() << std::endl;
		std::cout << "Has vertex colors: " << (mesh->HasVertexColors(0) ? "Yes" : "No") << std::endl;
#endif

		bool hasNormals = mesh->HasNormals();

		//std::cout << "UV channels: " << mesh->GetNumUVChannels() << std::endl;

		for (std::uint32_t vertIdx = 0u; vertIdx < mesh->mNumVertices; vertIdx++)
		{
			aiVector3D vert = mesh->mVertices[vertIdx];

			if (mesh->HasTangentsAndBitangents())
			{
				aiVector3D tan = mesh->mTangents[vertIdx];
				aiVector3D bit = mesh->mBitangents[vertIdx];
				nMesh->Tangents.push_back(aiVecToGlm(tan));
				nMesh->BiNormals.push_back(aiVecToGlm(bit));
			}

			nMesh->Vertices.push_back(aiVecToGlm(vert));


			if (hasNormals)
			{
				aiVector3D norm = mesh->mNormals[vertIdx];
				nMesh->Normals.push_back(aiVecToGlm(norm));
			}

			if (mesh->mTextureCoords[0])
			{
				nMesh->TexCoords.push_back(glm::vec2(mesh->mTextureCoords[0][vertIdx].x, mesh->mTextureCoords[0][vertIdx].y));
			}

			if (mesh->HasVertexColors(0))
			{

			}
		}

		for (std::uint32_t faceIdx = 0u; faceIdx < mesh->mNumFaces; faceIdx++)
		{
			nMesh->Indices.push_back(mesh->mFaces[faceIdx].mIndices[0u]);
			nMesh->Indices.push_back(mesh->mFaces[faceIdx].mIndices[1u]);
			nMesh->Indices.push_back(mesh->mFaces[faceIdx].mIndices[2u]);
		}

		//TODO: Bones
		/*if (mesh->HasBones())
		{
			std::cout << "Bones" << std::endl;

			std::vector<glm::vec4> weights;
			std::vector<glm::vec4> boneID;

			weights.resize(mesh->mNumVertices);
			boneID.resize(mesh->mNumVertices);

			std::fill(weights.begin(), weights.end(), glm::vec4(1.0f));
			std::fill(boneID.begin(), boneID.end(), glm::vec4(-123.0f));

			Animator* animator = rootAnimScene->getAnimator();
			BoneMap& boneIdMap = animator->BoneIdMap;

			for (int x = 0; x < mesh->mNumBones; x++)
			{
				unsigned int index = 0;

				if (boneIdMap.find(mesh->mBones[x]->mName.data) == boneIdMap.end())
				{ // create a new bone
					// current index is the new bone
					index = boneIdMap.size();
				}
				else
				{
					index = boneIdMap[mesh->mBones[x]->mName.data];
				}

				boneIdMap[mesh->mBones[x]->mName.data] = index;


				for (int y = 0; y < animator->Animations[animator->CurrentAnim].channels.size(); y++)
				{
					if (animator->Animations[animator->CurrentAnim].channels[y].name == mesh->mBones[x]->mName.data)
					{
						std::cout << "yyyyyyy" << std::endl;
					}
				}

			}
		}*/

		if (!hasNormals)
		{
			//nMesh->recalculateNormals();
		}

		return nMesh;
	}
}