// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TPMULTIPLAYER_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	virtual void StartPlay() override;
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override { return Player; }; // We managing spawns manually later, when player intends to join a game as a player or spectator

	virtual void Logout(AController* Exiting) override;

public:
	AActor* GetInitialSpawnLocationActor() { if (!SpectatorSpawn) SetupSpawnLocations(); return SpectatorSpawn; };

protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Setup")
	TSubclassOf<class AThirdPersonCharacter> GameplayPawnClass_RedTeam;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Setup")
	TSubclassOf<class AThirdPersonCharacter> GameplayPawnClass_BlueTeam;;

	static const FString NewPlayerOptionsNameKey; // When new PlayerController gets created, set its name from option parameter with that key name on AMainGameMode::Login() 

private:
	void SetupSpawnLocations();

	AActor* SpectatorSpawn;
	TArray<AActor*> TeamSpawns_Red;
	TArray<AActor*> TeamSpawns_Blue;
};
