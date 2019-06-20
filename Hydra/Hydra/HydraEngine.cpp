#include "Hydra/HydraEngine.h"
#include "Hydra/Render/Pipeline/View/MainRenderView.h"
#include "Hydra/Render/Pipeline/View/UIRenderView.h"

#include "Hydra/Framework/World.h"



#include "Hydra/Assets/Importers/ModelImporter.h"
#include "Hydra/Core/Stream/FileStream.h"

#include "GeneratedHeaders/HydraClassDatabase.generated.h"

// Test includes
#include "Hydra/Platform/WIndows/App/WinApplication.h"
#include "Hydra/Platform/WIndows/App/WinWindow.h"


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

	/*DeviceManager* deviceManager = DeviceManager::CreateDeviceManagerForPlatform();
	Context->SetDeviceManager(deviceManager);

	deviceManager->OnPrepareDeviceContext += EVENT_ARGS(HydraEngine, PrepareForEngineStart, DeviceCreationParameters&);
	deviceManager->OnDeviceDestroy += EVENT(HydraEngine, OnDestroy);

	deviceManager->AddVisualController(new MainRenderView(Context, this));
	deviceManager->AddVisualController(new UIRenderView(Context, this));

	World = new FWorld(Context);

	deviceManager->InitContext();*/

	SharedPtr<Application> app = MakeShared<WinApplication>();

	{
		SharedPtr<FWindowDefinition> winDefinition = MakeShared<FWindowDefinition>();

		winDefinition->AcceptsInput = true;
		winDefinition->ActivationPolicy = EWindowActivationPolicy::Always;
		winDefinition->AppearsInTaskbar = true;
		winDefinition->FocusWhenFirstShown = true;
		winDefinition->HasCloseButton = true;
		winDefinition->HasOSWindowBorder = true;
		winDefinition->HasSizingFrame = true;
		winDefinition->IsRegularWindow = true;
		winDefinition->Opacity = 0.5f;
		winDefinition->SupportsMaximize = true;
		winDefinition->SupportsMinimize = true;
		winDefinition->Title = "Yo";

		winDefinition->WidthDesiredOnScreen = 800;
		winDefinition->HeightDesiredOnScreen = 600;

		winDefinition->XDesiredPositionOnScreen = 1920 / 2 - winDefinition->WidthDesiredOnScreen / 2;
		winDefinition->YDesiredPositionOnScreen = 1080 / 2 - winDefinition->HeightDesiredOnScreen / 2;

		SharedPtr<FWindow> window = app->MakeWindow();
		app->InitializeWindow(window, winDefinition, nullptr, true);
	}

	{
		SharedPtr<FWindowDefinition> winDefinition = MakeShared<FWindowDefinition>();

		winDefinition->AcceptsInput = true;
		winDefinition->ActivationPolicy = EWindowActivationPolicy::Always;
		winDefinition->AppearsInTaskbar = true;
		winDefinition->FocusWhenFirstShown = true;
		winDefinition->HasCloseButton = true;
		winDefinition->HasOSWindowBorder = true;
		winDefinition->HasSizingFrame = true;
		winDefinition->IsRegularWindow = true;
		winDefinition->Opacity = 0.5f;
		winDefinition->SupportsMaximize = true;
		winDefinition->SupportsMinimize = true;
		winDefinition->Title = "Yo";

		winDefinition->WidthDesiredOnScreen = 800;
		winDefinition->HeightDesiredOnScreen = 600;

		winDefinition->XDesiredPositionOnScreen = 1920 / 2 - winDefinition->WidthDesiredOnScreen / 2 - 200;
		winDefinition->YDesiredPositionOnScreen = 1080 / 2 - winDefinition->HeightDesiredOnScreen / 2 - 200;


		winDefinition->Title = "Makarenis";

		winDefinition->AppearsInTaskbar = false;

		SharedPtr<FWindow> window = app->MakeWindow();
		app->InitializeWindow(window, winDefinition, nullptr, true);

	}

	MSG msg = { 0 };

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
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