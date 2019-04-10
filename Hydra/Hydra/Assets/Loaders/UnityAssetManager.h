#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/File.h"
#include "Hydra/Core/YamlConvertors.h"

namespace Hydra
{
	class Spatial;

	typedef String FileID;

	struct NodeDesc
	{
		int ObjectType;
		YAML::Node Node;
	};

	struct MetaInfo
	{
		String GUID;
	};

	struct FileIdInfo
	{
		String FileName;
		String GUID;
		Map<String, long> PartsIds;
	};

	class UnityAssetManager
	{
	private:
		File _AssetFolder;

		Map<String, FileIdInfo> _ModelIdData;

		Map<String, String> _FileGUIDMap;

		Map<String, SharedPtr<Spatial>> _PrefabsStringMap;
		Map<String, SharedPtr<Spatial>> _PrefabsGUIDMap;

		Map<String, Map<FileID, NodeDesc>> _NodesGUIDMap;
		Map<FileID, NodeDesc> _Nodes;

	public:
		UnityAssetManager(const File& assetFolder);

		void Init();

	private:

		void LoadModel(const File& file, bool postProcess = false);
		void LoadScene(const File& file, bool postProcess = false);
		void LoadMaterial(const File& file, bool postProcess = false);
		void LoadPrefab(const File& file, bool postProcess = false);

		void TryLoadTexture(const File& file);

		MetaInfo PreLoadBasicData(const File& file);

		void LoadFileIdInfo();

		MetaInfo LoadMetaDataForFile(const File& file);

		// For loading .unity and .prefab files
		Map<FileID, NodeDesc> LoadBaseFile(const File& file);
	};
}