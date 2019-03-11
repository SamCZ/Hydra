#if 1

#include <iostream>

#include "Hydra/Core/Common.h"
#include "Hydra/Core/Container.h"
#include "Hydra/Core/File.h"
#include "Hydra/Scene/Spatial.h"

#include "Hydra/Core/YamlConvertors.h"

using namespace Hydra;

typedef String FileID;

Map<FileID, YAML::Node> LoadBaseFile(const File& file)
{
	List<String> lines = file.ReadLines();

	Map<FileID, YAML::Node> nodes;

	String fixedYaml = "";
	FileID lastFileId = "";

	for (String& line : lines)
	{
		if (line.length() == 0) continue;

		if (StartsWith(line, "--- !u!"))
		{
			if (fixedYaml.length() > 0 && lastFileId.length() != 0)
			{
				nodes[lastFileId] = YAML::Load(fixedYaml);
				fixedYaml = "";
			}

			String ids = line.substr(7);
			List<String> idList = SplitString(ids, ' ');

			lastFileId = idList[1].substr(1);
		}
		else
		{
			fixedYaml += line + "\r\n";
		}
	}

	

	return nodes;
}

YAML::Node LoadMetaFile(const File& file)
{
	return YAML::LoadFile(file.GetPath());
}

int main()
{
	File baseFolder = File("D:\\Sam\\Projekty\\Unity\\IndustryEmpire\\Assets\\");

	Map<FileID, YAML::Node> nodes = LoadBaseFile(File(baseFolder, "MainScene.unity"));
	YAML::Node meta = LoadMetaFile(File(baseFolder, "MainScene.unity.meta"));

	Map<FileID, SpatialPtr> sceneNodes;
	Map<FileID, SpatialPtr> transformNodeMap;

	SpatialPtr rootNode = MakeShared<Spatial>("RootNode");

	for (int i = 0; i < 2; i++)
	{
		ITER(nodes, it)
		{
			FileID fileId = it->first;
			YAML::Node nodeRaw = it->second;
			String key = nodeRaw.begin()->first.as<String>();

			if (key == "GameObject")
			{
				YAML::Node node = nodeRaw["GameObject"];

				String name = node["m_Name"].as<String>();
				bool isActive = node["m_IsActive"].as<int>() == 1;
				YAML::Node componentsNode = node["m_Component"];

				//std::cout << name << std::endl;

				SpatialPtr spatial = nullptr;

				// Fist create nodes
				if (i == 0)
				{
					spatial = MakeShared<Spatial>(name);
					spatial->SetEnabled(isActive);
					sceneNodes[fileId] = spatial;
				}
				else
				{
					spatial = sceneNodes[fileId];
				}

				// Then create components and join nodes
				if (true)
				{
					for (YAML::const_iterator it = componentsNode.begin(); it != componentsNode.end(); ++it)
					{
						FileID componentId = (*it)["component"].begin()->second.as<String>();

						if (nodes.find(componentId) != nodes.end())
						{
							YAML::Node componentNodeRaw = nodes[componentId];
							String componentKey = componentNodeRaw.begin()->first.as<String>();

							//std::cout << " - " << componentKey << std::endl;

							if (componentKey == "Transform")
							{
								transformNodeMap[componentId] = spatial;

								if (i == 1)
								{
									YAML::Node componentNode = componentNodeRaw["Transform"];

									spatial->Position = componentNode["m_LocalPosition"].as<Vector3>();
									spatial->Scale = componentNode["m_LocalScale"].as<Vector3>();

									//m_LocalEulerAnglesHint can be used as rotation in angles but i dont't know if it works at any circumstances.
									Vector4 rotationQuaternion = componentNode["m_LocalRotation"].as<Vector4>();
									Quaternion quat = Quaternion(rotationQuaternion.w, rotationQuaternion.x, rotationQuaternion.y, rotationQuaternion.z);
									spatial->Rotation = glm::degrees(glm::eulerAngles(quat));

									YAML::Node childrenMapNode = componentNode["m_Children"];

									for (YAML::const_iterator it2 = childrenMapNode.begin(); it2 != childrenMapNode.end(); ++it2)
									{
										FileID childComponentId = (*it2)["fileID"].as<String>();

										if (transformNodeMap.find(childComponentId) != transformNodeMap.end())
										{
											spatial->AddChild(transformNodeMap[childComponentId]);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	ITER(sceneNodes, it)
	{
		SpatialPtr spatial = it->second;

		if (spatial->Parent == nullptr)
		{
			rootNode->AddChild(spatial);
		}
	}

	rootNode->PrintHiearchy();

	YAML::Node mappingNode = YAML::LoadFile(File(baseFolder, "FileAssetMap.yml").GetPath());

	for (YAML::const_iterator it = mappingNode.begin(); it != mappingNode.end(); ++it)
	{
		String name = (*it).first.as<String>();

		std::cout << name << std::endl;

		YAML::Node childNode = mappingNode[name];

		for (YAML::const_iterator it2 = childNode.begin(); it2 != childNode.end(); ++it2)
		{
			String name2 = (*it2).first.as<String>();
			String name3 = (*it2).second.as<String>();

			std::cout << " - " << name2 << ": " << name3 << std::endl;
		}
	}

	while (true);

	return 0;
}

#endif