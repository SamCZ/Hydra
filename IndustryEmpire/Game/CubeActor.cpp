#include "CubeActor.h"

#include "Hydra/EngineContext.h"

void ACubeActor::InitializeComponents()
{
	CubeComponent = AddComponent<HStaticMeshComponent>("Cube");
	CubeComponent->StaticMesh = Engine->GetAssetManager()->GetMesh("Assets/BasicShapes/Cube.FBX");
}

void ACubeActor::BeginPlay()
{
	Vector2 Position = ((Vector2)Engine->ScreenSize) * 0.5f;
	//SetLocation(Position.x, Position.y, 0);

	Velocity = Rnd.GetRandomUnitVector3() * 10.0f;
	Acceleration = Vector2(1.005f, 1.005f);
}

void ACubeActor::Tick(float DeltaTime)
{
	//Velocity *= Acceleration;
	//AddLocation(Velocity.x, Velocity.y, 0);


	if (GetLocation().x > Engine->ScreenSize.x || GetLocation().x <= 0)
	{
		Velocity.x *= -1;
	}

	if (GetLocation().y > Engine->ScreenSize.y || GetLocation().y <= 0)
	{
		Velocity.y *= -1;
	}
}

void ACubeActor::OnHud(UIRenderer * renderer)
{
	Vector3 location = GetLocation();

	renderer->DrawOval(location.x, location.y, 10, 10, ColorRGBA::Blue);

	//NVRHI::TextureHandle tex = Engine->GetAssetManager()->GetTexture("Assets/Textures/Metal/copper-rock1-alb.png");
	//renderer->DrawImage(tex, location.x, location.y, 50, 50, 0);
}
