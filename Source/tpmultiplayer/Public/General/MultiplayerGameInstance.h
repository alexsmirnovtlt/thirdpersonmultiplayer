// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MultiplayerGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class TPMULTIPLAYER_API UMultiplayerGameInstance : public UGameInstance
{
	GENERATED_BODY()

public: 
	int32 GetMaxOnlinePlayers() { return MaxOnlinePlayers; };
	FString& GetLobbyMapName() { return LobbyMapName; };
	FString& GetGameplayMapNameForHost() { return MapNameForHost; };

private:
	int32 MaxOnlinePlayers = 6;
	FString LobbyMapName = "/Game/Maps/MainMenu";
	FString MapNameForHost = "/Game/Maps/GameplayLevel?listen";
};
