// Aleksandr Smirnov 2021


#include "General/GameModes/LobbyGameMode.h"

#include "Widgets/SWeakWidget.h"

#include "Slate/SLobbyWidget.h"

void ALobbyGameMode::StartPlay()
{
	Super::StartPlay();

	// Some checks before Slate Widget creation 
	if (!GEngine || (GEngine && !GEngine->GameViewport)) return;
	if (!MenuStyleClass) { UE_LOG(LogTemp, Error, TEXT("ALobbyGameMode: MenuStyleClass must be assigned!")); return; };

	// Getting widget style info that we specified in blueprint. Will be used in SLobbyWidget::Construct()
	FLobbyMenuSlateStyle& Style = MenuStyleClass.GetDefaultObject()->WidgetStyle;

	// Creating Lobby Slate Widget and passing parameters to it
	LobbyWidget = SNew(SLobbyWidget).LobbyGameMode(this).LobbyStyle(&Style);
	
	// Placing SLobbyWidget in a WeakPtr container so when this GameMode gets destroyed, TSharedPtr LobbyWidget will decrement one reference and LobbyWidget will be destroyed
	GEngine->GameViewport->AddViewportWidgetContent(
		SAssignNew(LobbyWidgetContainer, SWeakWidget)
		.PossiblyNullContent(LobbyWidget)
	);

	//

	// Showing mouse cursor when fullscreen
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		FInputModeUIOnly InputModeData = FInputModeUIOnly();
		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = true;
	}
}

AActor* ALobbyGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	return Player; // Instead of looking for PlayerStart actors just get Controller`s location (should be FVector::ZeroVector)
}

void ALobbyGameMode::OnButtonPress_Host()
{


}
void ALobbyGameMode::OnButtonPress_Join()
{

}