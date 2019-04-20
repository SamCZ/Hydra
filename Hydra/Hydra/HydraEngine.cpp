#include "Hydra/HydraEngine.h"
#include "Hydra/Render/Pipeline/View/MainRenderView.h"
#include "Hydra/Render/Pipeline/View/UIRenderView.h"

#include "Hydra/Framework/World.h"



#include "Hydra/Assets/Importers/ModelImporter.h"
#include "Hydra/Core/Stream/FileStream.h"

HydraEngine::~HydraEngine()
{
	delete Context;
}

void HydraEngine::Start()
{
	Context = new EngineContext();

	DeviceManager* deviceManager = DeviceManager::CreateDeviceManagerForPlatform();
	Context->SetDeviceManager(deviceManager);

	deviceManager->OnPrepareDeviceContext += EVENT_ARGS(HydraEngine, PrepareForEngineStart, DeviceCreationParameters&);
	deviceManager->OnDeviceDestroy += EVENT(HydraEngine, OnDestroy);

	deviceManager->AddVisualController(new MainRenderView(Context, this));
	deviceManager->AddVisualController(new UIRenderView(Context, this));

	World = new FWorld(Context);

	AssetManager* assetManager = new AssetManager();
	Context->SetAssetManager(assetManager);
	InitializeAssetManager(assetManager);

	deviceManager->InitContext();
}

void HydraEngine::OnDestroy()
{
	delete Context->GetAssetManager();
	Context->SetAssetManager(nullptr);
}

void HydraEngine::PrepareForEngineStart(DeviceCreationParameters& params)
{
	Context->ScreenSize = Vector2i(params.Width, params.Height);
}

void HydraEngine::InitializeAssetManager(AssetManager* assetManager)
{
	ModelImporter modelImporter;

	File modelFile = File("Assets/IndustryEmpire/Models/trees_v1.fbx");

	FileStream stream = FileStream(modelFile);

	Blob* data = stream.Read();

	HAsset* asset = nullptr;
	modelImporter.Import(*data, ModelImportOptions(), asset);
	 
	delete data;
}

FWorld* HydraEngine::GetWorld() const
{
	return World;
}

EngineContext* HydraEngine::GetContext() const
{
	return Context;
}