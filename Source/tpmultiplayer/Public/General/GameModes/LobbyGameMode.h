// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interfaces/OnlineSessionInterface.h"

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

	AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;

	void OnStartHosting(FText& SessionName, FText& PlayerName);
	void OnStartSearchingGames();
	void OnStartJoining(FText& SessionName, FText& PlayerName, int32 SessionIndex);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Slate Styling")
	TSubclassOf<class ULobbyMenuSlateWidgetStyle> MenuStyleClass;
	UPROPERTY(EditDefaultsOnly, Category = "Slate Styling")
	TSubclassOf<class ULobbyFoundGameInfoWidgetStyle> SessionItemStyleClass;

	UPROPERTY(EditDefaultsOnly, Category = "OnlineSybsystem")
	int32 MaxSearchResults = 10;

	TSharedPtr<class SLobbyWidget> LobbyWidget;
	TSharedPtr<class SWeakWidget> LobbyWidgetContainer;

	void InitOnlineSubsystem();
	void CreateMainWidget();

	// Begin Online Sybsystem

	TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe> SessionInterface;

	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type SessionType);
	void OnDestroySessionComplete(FName SessionName, bool Success);

	TSharedPtr<class FOnlineSessionSearch> SessionSearchResults;

	static FName CreatedSessionName;
	static const FName SERVER_NAME_SETTINGS_KEY;

	// End Online Sybsystem
};
