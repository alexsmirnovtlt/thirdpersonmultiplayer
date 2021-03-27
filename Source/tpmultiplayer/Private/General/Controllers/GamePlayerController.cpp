// Aleksandr Smirnov 2021


#include "General/Controllers/GamePlayerController.h"

#include "General/MultiplayerGameInstance.h"
#include "General/HUD/GameplayHUD.h"

void AGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	GameplayHUD = GetHUD<AGameplayHUD>();

	// Showing mouse cursor because HUD will spawn a menu
	FInputModeUIOnly InputModeData = FInputModeUIOnly();
	this->SetInputMode(InputModeData);
	this->bShowMouseCursor = true;
}

void AGamePlayerController::JoinGameAsPlayer()
{
	GameplayHUD->MainMenu_Hide();
}

void AGamePlayerController::JoinGameAsSpectator()
{
	GameplayHUD->MainMenu_Hide();
}

void AGamePlayerController::ReturnToLobby()
{
	auto GameInstance = CastChecked<UMultiplayerGameInstance>(GetWorld()->GetGameInstance());
	FString& LobbyMapName = GameInstance->GetLobbyMapName();

	if (HasAuthority())
	{
		if (GetWorld()->IsPlayInEditor())
		{
			// Otherwise GEngine->Browse() crashes when PIE
			GetWorld()->ServerTravel(LobbyMapName, true);
		}
		else
		{
			auto WorldContext = GetGameInstance()->GetWorldContext();
			FURL URL = FURL(*LobbyMapName);
			FString ErrorStr;

			GEngine->Browse(*WorldContext, URL, ErrorStr);
			// Session will be closed on return to lobby
		}
	}
	else
	{
		ClientTravel(LobbyMapName, ETravelType::TRAVEL_Absolute);
	}
}