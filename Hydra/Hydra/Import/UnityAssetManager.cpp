#include "Hydra/Import/UnityAssetManager.h"

#include "Hydra/Scene/Spatial.h"

#include "Hydra/Import/MeshImporter.h"

namespace Hydra
{
	UnityAssetManager::UnityAssetManager(const File& assetFolder) : _AssetFolder(assetFolder)
	{

	}

	void UnityAssetManager::Init()
	{
		_Nodes.clear();

		LoadFileIdInfo();

		List<File> files = _AssetFolder.ListFiles();

		for (int i = 0; i < 2; i++)
		{
			bool postProcess = i == 1;

			for (File file : files)
			{
				String ext = file.GetExtension();

				if (ext == "fbx")
				{
					LoadModel(file, postProcess);
				}
				else if (ext == "unity")
				{
					LoadScene(file, postProcess);
				}
				else if (ext == "mat")
				{
					LoadMaterial(file, postProcess);
				}
				else if (ext == "prefab")
				{
					LoadPrefab(file, postProcess);
				}
				else
				{
					TryLoadTexture(file);
				}
			}
		}

		std::cout << "Node count: " << _Nodes.size() << std::endl;
	}

	void UnityAssetManager::LoadModel(const File& file, bool postProcess)
	{
		if (postProcess) return;
		
		String localPath = file.GetPathAt("Assets").GetPath();


		MetaInfo metaData = LoadMetaDataForFile(file);
		SpatialPtr model = Meshimporter::Import(file, MeshImportOptions());

		model->GUID = metaData.GUID;

		// Map unity fileID to model
		if (_ModelIdData.find(localPath) != _ModelIdData.end())
		{
			FileIdInfo& info = _ModelIdData[localPath];

			ITER(info.PartsIds, it)
			{
				SpatialPtr child = model->Find(it->first);

				if (child != nullptr)
				{
					child->FileID = it->second;
				}
			}
		}

		// TODO: save model to further processing
	}

	void UnityAssetManager::LoadScene(const File& file, bool postProcess)
	{
		if (!postProcess)
		{
			std::cout << "PreLoading scene: " << file << std::endl;

			PreLoadBasicData(file);

			// TODO: create instances of spatial
			return;
		}

		std::cout << "Initializing scene: " << file << std::endl;

		String GUID = _FileGUIDMap[file.GetPath()];

		// TODO: read components and assign childs
		// TODO: make main scene
	}

	void UnityAssetManager::LoadMaterial(const File& file, bool postProcess)
	{
		// TODO: Load material
	}

	void UnityAssetManager::LoadPrefab(const File& file, bool postProcess)
	{
		if (!postProcess)
		{
			std::cout << "PreLoading prefab: " << file << std::endl;

			PreLoadBasicData(file);

			// TODO: create instances of spatial
			return;
		}

		std::cout << "Initializing prefab: " << file << std::endl;

		String GUID = _FileGUIDMap[file.GetPath()];

		// TODO: read components and assign childs
	}

	void UnityAssetManager::TryLoadTexture(const File & file)
	{
		// TODO: Load textures
	}

	MetaInfo UnityAssetManager::PreLoadBasicData(const File& file)
	{
		MetaInfo metaData = LoadMetaDataForFile(file);
		Map<FileID, NodeDesc> fileData = LoadBaseFile(file);

		if (_FileGUIDMap.find(file.GetPath()) == _FileGUIDMap.end())
		{
			_FileGUIDMap[file.GetPath()] = metaData.GUID;
		}

		if (_NodesGUIDMap.find(metaData.GUID) != _NodesGUIDMap.end())
		{
			LogError("UnityAssetManager::PreLoadBasicData", file.GetPath(), "Error duplicity GUID ids " + metaData.GUID + " !");
			return metaData;
		}

		_NodesGUIDMap[metaData.GUID] = fileData;

		ITER(fileData, it)
		{
			FileID id = it->first;
			NodeDesc& desc = it->second;

			if (_Nodes.find(id) != _Nodes.end())
			{
				LogError("UnityAssetManager::PreLoadBasicData", file.GetPath(), "Error duplicity fileID ids " + id + " !");
				continue;
			}

			_Nodes[id] = desc;
		}

		return metaData;
	}

	void UnityAssetManager::LoadFileIdInfo()
	{
		_ModelIdData.clear();

		YAML::Node mappingNode = YAML::LoadFile(File(_AssetFolder, "FileAssetMap.yml").GetPath());

		for (YAML::const_iterator it = mappingNode.begin(); it != mappingNode.end(); ++it)
		{
			String name = (*it).first.as<String>();

			FileIdInfo fileInfo = {};
			fileInfo.FileName = name;

			YAML::Node childNode = mappingNode[name];

			for (YAML::const_iterator it2 = childNode.begin(); it2 != childNode.end(); ++it2)
			{
				String key = (*it2).first.as<String>();
				String val = (*it2).second.as<String>();

				if (key == "GUID")
				{
					fileInfo.GUID = val;
				}
				else
				{
					fileInfo.PartsIds[key] = std::atol(val.c_str());
				}
			}

			_ModelIdData[name] = fileInfo;
		}
	}

	MetaInfo UnityAssetManager::LoadMetaDataForFile(const File& file)
	{
		MetaInfo data;

		File metaFile = File(file.GetPath() + ".meta");

		YAML::Node node = YAML::LoadFile(metaFile.GetPath());

		data.GUID = node["guid"].as<String>();

		return data;
	}

	Map<FileID, NodeDesc> UnityAssetManager::LoadBaseFile(const File & file)
	{
		List<String> lines = file.ReadLines();

		Map<FileID, NodeDesc> nodes;

		String fixedYaml = "";
		int lastObjId = -1;
		FileID lastFileId = "";

		for (String& line : lines)
		{
			if (line.length() == 0) continue;

			if (StartsWith(line, "--- !u!"))
			{
				if (fixedYaml.length() > 0 && lastFileId.length() != 0)
				{
					NodeDesc desc = {};
					desc.Node = YAML::Load(fixedYaml);
					desc.ObjectType = lastObjId;

					nodes[lastFileId] = desc;
					fixedYaml = "";
				}

				String ids = line.substr(7);
				List<String> idList = SplitString(ids, ' ');
				lastObjId = std::atoi(idList[0].c_str());
				lastFileId = idList[1].substr(1);
			}
			else
			{
				fixedYaml += line + "\r\n";
			}
		}

		return nodes;
	}
}