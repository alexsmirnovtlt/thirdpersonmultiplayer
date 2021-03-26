// Aleksandr Smirnov 2021


#include "General/HUD/GameplayHUD.h"

#include "Widgets/SWeakWidget.h"

#include "General/Controllers/GamePlayerController.h"
#include "Slate/GameplayMainMenuWidgetStyle.h"
#include "Slate/GameplayMainMenu.h"

void AGameplayHUD::BeginPlay()
{
	auto PC = CastChecked<AGamePlayerController>(GetOwningPlayerController());

	// Creating and showing main menu widget so player can join a game or return to lobby

	if (!GEngine || (GEngine && !GEngine->GameViewport)) return;
	if (!MainMenuStyleClass) { UE_LOG(LogTemp, Error, TEXT("AGameplayHUD: Defaults must be assigned!")); return; };

	MainMenuWidget = SNew(SGameplayMainMenuWidget).PlayerController(PC).MainMenuStyle(MainMenuStyleClass.GetDefaultObject());

	GEngine->GameViewport->AddViewportWidgetContent(
		SAssignNew(MainMenuWidgetContainer, SWeakWidget)
		.PossiblyNullContent(MainMenuWidget)
	);
}