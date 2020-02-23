#include "Hydra/HydraEngine.h"
#include "Hydra/Render/Pipeline/View/MainRenderView.h"
#include "Hydra/Render/Pipeline/View/UIRenderView.h"

#include "Hydra/Framework/World.h"



#include "Hydra/Assets/Importers/ModelImporter.h"
#include "Hydra/Core/Stream/FileStream.h"

#include "GeneratedHeaders/HydraClassDatabase.generated.h"

// Test includes
#include "Hydra/Platform/Windows/App/WinApplication.h"
#include "Hydra/Platform/Windows/App/WinWindow.h"

#include "Hydra/App/WindowManager.h"

#include "Hydra/App/UI/Widgets/GameViewWidget.h"


#include "Hydra/Input/Windows/WindowsInputManager.h"

HydraEngine* GEngine = nullptr;

HydraEngine::~HydraEngine()
{
	delete Context;
}

HydraEngine::HydraEngine()
{
	Hydra_InitializeClassDatabase();
}

void HydraEngine::Start()
{
	Context = new EngineContext();

	GEngine = this;

	/*DeviceManager* deviceManager = DeviceManager::CreateDeviceManagerForPlatform();
	Context->SetDeviceManager(deviceManager);

	deviceManager->OnPrepareDeviceContext += EVENT_ARGS(HydraEngine, PrepareForEngineStart, DeviceCreationParameters&);
	deviceManager->OnDeviceDestroy += EVENT(HydraEngine, OnDestroy);

	deviceManager->AddVisualController(new MainRenderView(Context, this));
	deviceManager->AddVisualController(new UIRenderView(Context, this));

	World = new FWorld(Context);

	deviceManager->InitContext();*/


	SharedPtr<Application> app = MakeShared<WinApplication>();

	World = new FWorld(Context);

	GEngine->GetContext()->ScreenSize = Vector2(1280, 720);

	app->Initialize();

	app->InitializeEngineContext(Context);


	{
		SharedPtr<UIWindow> window = UINew(UIWindow)
			.Type(EWindowType::Normal)
			.CreateTitleBar(true)
			.FocusWhenFirstShown(true)
			.HasCloseButton(true)
			.InitialOpacity(1.0f)
			.SupportsMaximize(true)
			.SupportsMinimize(true)
			.Title("Hydra")
			.UseOSWindowBorder(true)
			[
				UINew(GameViewWidget)
			];

		WindowManager::Get().AddWindow(window, true);
	}

	app->Run();
}

void HydraEngine::OnDestroy()
{
	if (World)
	{
		delete World;
		World = nullptr;
	}
}

void HydraEngine::PrepareForEngineStart(DeviceCreationParameters& params)
{
#if WITH_EDITOR
	params.Maximized = true;
#endif

	Context->ScreenSize = Vector2i(params.Width, params.Height);
}

void HydraEngine::InitializeAssetManager(AssetManager* assetManager)
{
	/*ModelImporter modelImporter;

	File modelFile = File("Assets/IndustryEmpire/Models/trees_v1.fbx");

	FileStream stream = FileStream(modelFile);

	Blob* data = stream.Read();

	List<HAsset*> assets;

	ModelImportOptions options;
	options.CombineMeshes = true;
	modelImporter.Import(*data, options, assets);

	Log("Loaded meshes", ToString(assets.size()));

	for (HAsset* asset : assets)
	{
		delete asset;
	}

	delete data;*/
}

FWorld* HydraEngine::GetWorld() const
{
	return World;
}

EngineContext* HydraEngine::GetContext() const
{
	return Context;
}