// Aleksandr Smirnov 2021


#include "General/HUD/GameplayHUD.h"

#include "Widgets/SWeakWidget.h"

#include "General/Controllers/GamePlayerController.h"
#include "Slate/GameplayMainMenuWidgetStyle.h"
#include "Slate/GameplayMainMenu.h"

void AGameplayHUD::BeginPlay()
{
	Super::BeginPlay();
	
	MainMenu_Show(); // Creating and showing main menu widget so player can join a game or return to lobby
}

void AGameplayHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	MainMenu_Hide(); // If HUD got somehow destroyed while menu was open, we need to explicitly remove WeakPtr from GEngine->GameViewport

	Super::EndPlay(EndPlayReason);
}

void AGameplayHUD::MainMenu_Show()
{
	auto PC = CastChecked<AGamePlayerController>(GetOwningPlayerController());

	if (!GEngine || (GEngine && !GEngine->GameViewport)) return;
	if (!MainMenuStyleClass) { UE_LOG(LogTemp, Error, TEXT("AGameplayHUD: Defaults must be assigned!")); return; };

	if (!MainMenuWidget.IsValid())
		MainMenuWidget = SNew(SGameplayMainMenuWidget).PlayerController(PC).MainMenuStyle(MainMenuStyleClass.GetDefaultObject());

	GEngine->GameViewport->AddViewportWidgetContent(
		SAssignNew(MainMenuWidgetContainer, SWeakWidget)
		.PossiblyNullContent(MainMenuWidget)
	);
}

void AGameplayHUD::MainMenu_Hide()
{
	if (!MainMenuWidget.IsValid()) return;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(MainMenuWidgetContainer.ToSharedRef());

		MainMenuWidgetContainer.Reset();
		MainMenuWidget.Reset();
	}
}