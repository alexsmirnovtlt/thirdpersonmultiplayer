// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainGameMode.generated.h"

/**
 * Handles login and logout of PlayerControllersMain and main server game logic
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

public:
	void AddPlayerToAMatch(class AGamePlayerController* PlayerController);
	void RemovePlayerFromAMatch(class AGamePlayerController* PlayerController);

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Classes")
	TSubclassOf<class AThirdPersonCharacter> GameplayPawnClass_RedTeam;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Classes")
	TSubclassOf<class AThirdPersonCharacter> GameplayPawnClass_BlueTeam;

	AActor* SpectatorSpawn;
	TArray<AActor*> TeamSpawns_Red;
	TArray<AActor*> TeamSpawns_Blue;

	TArray<class AThirdPersonCharacter*> TeamPawns_Red;
	TArray<class AThirdPersonCharacter*> TeamPawns_Blue;

	int32 HumanPlayersCount_RedTeam = 0;
	int32 HumanPlayersCount_BlueTeam = 0;

	void SetupSpawnLocations(); // Getting all PlayerStarts from level to have locations for players to spawn
	void SetupPlayableCharacters(); // Happend once on BeginPlay() - occupies all available spawn points with AI controlled characters so new players can possess them or just watch

	class AGameplayGameState* GameplayState;

	static const FString NewPlayerOptionsNameKey; // When new PlayerController gets created, set its name from option parameter with that key name on AMainGameMode::Login() 
};
