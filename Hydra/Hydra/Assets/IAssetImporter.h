#pragma once

#include "Hydra/Core/Stream/Blob.h"
#include "Asset.h"
#include "Hydra/Core/Log.h"
#include "Hydra/Framework/Object.h"

class HYDRA_API AssetImportOptions : public HObject
{
	HCLASS_BODY(AssetImportOptions)
};

class IAssetImporter
{
public:
	virtual bool Import(Blob& dataBlob, const AssetImportOptions& options, HAsset*& out_Asset) = 0;
};