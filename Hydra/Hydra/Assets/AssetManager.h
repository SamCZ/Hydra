#pragma once

#include "Hydra/Core/Common.h"

#include "IAssetImporter.h"
#include "IAssetLocator.h"

class HYDRA_API AssetManager
{
private:

public:
	AssetManager();
	~AssetManager();

	void AddAssetLocator(IAssetLocator* locator);
	void AddAssetImporter(IAssetImporter* importer);
};