#if 0

#include <iostream>

#include "Hydra/Import/UnityAssetManager.h"

using namespace Hydra;

int main()
{
	File baseFolder = File("D:\\Sam\\Projekty\\C++\\Hydra\\UnityEditorProject\\Assets");

	UnityAssetManager asman = UnityAssetManager(baseFolder);

	asman.Init();


	/*

	

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

	*/

	while (true);

	return 0;
}

#endif