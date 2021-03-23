// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "Slate/LobbyMenuSlateWidgetStyle.h"

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

	void OnButtonPress_Host();
	void OnButtonPress_Join();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Slate Styling")
	TSubclassOf<class ULobbyMenuSlateWidgetStyle> MenuStyleClass;

	TSharedPtr<class SLobbyWidget> LobbyWidget;
	TSharedPtr<class SWeakWidget> LobbyWidgetContainer;
};
