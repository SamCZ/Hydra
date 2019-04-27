#pragma once

#include "Hydra/Assets/IAssetImporter.h"

class TextureImporter
{
public:
	bool Import(Blob& dataBlob, const AssetImportOptions& options, List<HAsset*>& out_Assets);
};