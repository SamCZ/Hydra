#pragma once

#include "Hydra/Core/Common.h"
#include "Hydra/Core/Delegate.h"

#include "IAssetImporter.h"
#include "IAssetLocator.h"

#include "Hydra/Core/File.h"

#include "Hydra/Render/Material.h"

#include "Hydra/Render/Pipeline/GFSDK_NVRHI.h"

class EngineContext;
class Technique;
class HStaticMesh;

class HYDRA_API AssetManager
{
private:
	EngineContext* _Context;

	Map<String, SharedPtr<Technique>> _Techniques;
	Map<String, MaterialInterface*> _Materials;
	Map<String, NVRHI::TextureHandle> _Textures;

	List<HStaticMesh*> _TemporalStaticMeshContainer;
	Map<String, List<HStaticMesh*>> _TemportalStaticMeshMap;
public:
	DelegateEvent<void, HStaticMesh*> OnMeshLoaded;
	DelegateEvent<void, HStaticMesh*> OnMeshDeleted;

	AssetManager(EngineContext* context);
	~AssetManager();

	void LoadProjectFiles();

	void AddAssetLocator(IAssetLocator* locator);
	void AddAssetImporter(IAssetImporter* importer);

	// BLOCK(This whole section in c++ will be redone) begin

	MaterialInterface* GetMaterial(const String& path);
	NVRHI::TextureHandle GetTexture(const String& path);

	HStaticMesh* GetMesh(const String& path);
	List<HStaticMesh*> GetMeshParts(const String path);

	// BLOCK end
private:

	SharedPtr<Technique> LoadTechnique(const File& file); // TODO: These methods are only temporal, we need to create methods or importers that are compatible with compressed or hashed files.
	void LoadMaterial(const File& file);
	void SaveMaterial(MaterialInterface* material);
	void LoadTexture(const File& file);
};