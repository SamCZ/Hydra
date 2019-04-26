#pragma once

#include "Hydra/Core/Common.h"

#include "IAssetImporter.h"
#include "IAssetLocator.h"

#include "Hydra/Core/File.h"

#include "Hydra/Render/Material.h"

class EngineContext;
class Technique;

class HYDRA_API AssetManager
{
private:
	EngineContext* _Context;

	Map<String, SharedPtr<Technique>> _Techniques;
	Map<String, MaterialInterface*> _Materials;
public:
	AssetManager(EngineContext* context);
	~AssetManager();

	void LoadProjectFiles();

	void AddAssetLocator(IAssetLocator* locator);
	void AddAssetImporter(IAssetImporter* importer);

	MaterialInterface* GetMaterial(const String& path);
private:

	SharedPtr<Technique> LoadTechnique(const File& file); // TODO: These methods are only temporal, we need to create methods or importers that are compatible with compressed or hashed files.
	void LoadMaterial(const File& file);
	void SaveMaterial(MaterialInterface* material);
};