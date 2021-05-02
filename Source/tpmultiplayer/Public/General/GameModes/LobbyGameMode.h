// Aleksandr Smirnov 2021

#pragma once

#include "FMODBank.h"
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "FMODEvent.h"

#include "LobbyGameMode.generated.h"

/**
 * Startup Gamemode that handles Host or Join a game by a player
 */
UCLASS()
class TPMULTIPLAYER_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void StartPlay() override;
	virtual void Logout(AController* Exiting) override;
	
	AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;

	void OnStartHosting(FText& SessionName);
	void OnStartSearchingGames();
	void OnStartJoining(FText& SessionName, int32 SessionIndex);

	void PlayButtonClickSound();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Slate Styling")
	TSubclassOf<class ULobbyMenuSlateWidgetStyle> MenuStyleClass;
	UPROPERTY(EditDefaultsOnly, Category = "Slate Styling")
	TSubclassOf<class ULobbyFoundGameInfoWidgetStyle> SessionItemStyleClass;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "FMOD Audio")
	UFMODEvent* ButtonClickSound;

	UPROPERTY(EditDefaultsOnly, Category = "OnlineSubsystem")
	int32 MaxSearchResults = 10;

	UPROPERTY(EditDefaultsOnly, Category = "FMOD Audio")
	TArray<UFMODBank*> LobbyFMODBanks;
	UPROPERTY(EditDefaultsOnly, Category = "FMOD Audio")
	TArray<UFMODBank*> MainGameMapFMODBanks;

	TSharedPtr<class SLobbyWidget> LobbyWidget;
	TSharedPtr<class SWeakWidget> LobbyWidgetContainer;

	void InitOnlineSubsystem();
	void CreateMainWidget();

	void FMOD_HandleBanksOnMapChange();

	// FMOD Related
	void LoadFMODBanks(TArray<UFMODBank*>& ArrayToLoad);
	void UnloadFMODBanks(TArray<UFMODBank*>& ArrayToUnload);
	//

	// Begin Online Sybsystem

	TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe> SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearchResults;

	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type SessionType);
	void OnDestroySessionComplete(FName SessionName, bool Success);

	static FName CreatedSessionName;
	static const FName SERVER_NAME_SETTINGS_KEY;

	// End Online Sybsystem
};
