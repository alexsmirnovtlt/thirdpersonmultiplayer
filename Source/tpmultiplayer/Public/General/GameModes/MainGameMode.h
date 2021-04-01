// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainGameMode.generated.h"

/**
 * Handles only Login() and Logout() of PlayerControllers. Main game logic located in AGameplayGameState
 */
UCLASS()
class TPMULTIPLAYER_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	virtual void StartPlay() override;
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override { return Player; }; // We managing spawns differently, when player wants to posess a pawn, it is already somewhere in a game

	virtual void Logout(AController* Exiting) override;

protected:
	static const FString NewPlayerOptionsNameKey; // When new PlayerController gets created, set its name from option parameter with that key name on AMainGameMode::Login() 
};
