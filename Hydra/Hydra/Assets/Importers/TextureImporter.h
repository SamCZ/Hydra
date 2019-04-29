#pragma once

#include "Hydra/Assets/IAssetImporter.h"
#include "Hydra/EngineContext.h"

class TextureImporter
{
private:
	EngineContext* _Context;
public:
	TextureImporter(EngineContext* context);

	bool Import(Blob& dataBlob, const AssetImportOptions& options, List<HAsset*>& out_Assets);
};