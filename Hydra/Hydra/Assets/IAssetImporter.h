#pragma once

#include "Hydra/Core/Stream/Blob.h"
#include "Asset.h"
#include "Hydra/Core/Log.h"
#include "Hydra/Framework/Object.h"

HCLASS()
class HYDRA_API AssetImportOptions : public HObject
{
	HCLASS_GENERATED_BODY()
};

class IAssetImporter
{
public:
	virtual bool Import(Blob& dataBlob, const AssetImportOptions& options, List<HAsset*>& out_Assets) = 0;
};