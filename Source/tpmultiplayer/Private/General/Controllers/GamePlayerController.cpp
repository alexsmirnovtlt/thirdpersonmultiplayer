// Aleksandr Smirnov 2021


#include "General/Controllers/GamePlayerController.h"

#include "General/MultiplayerGameInstance.h"

void AGamePlayerController::JoinGameAsPlayer()
{

}

void AGamePlayerController::JoinGameAsSpectator()
{

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
			auto worldContext = GetGameInstance()->GetWorldContext();
			const TCHAR* textURL = *LobbyMapName;
			FURL url = FURL(nullptr, textURL, ETravelType::TRAVEL_Absolute);
			FString errorStr;

			// Session will be closed on return to lobby
			GEngine->Browse(*worldContext, url, errorStr);
		}
	}
	else
	{
		ClientTravel(LobbyMapName, ETravelType::TRAVEL_Absolute);
	}
}