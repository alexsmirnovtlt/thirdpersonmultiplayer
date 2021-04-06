// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainGameMode.generated.h"

enum class ETeamType : uint8;

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
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Classes")
	TSubclassOf<class AGameplayAIController> AIControllerClass;

	AActor* SpectatorSpawn;
	TArray<class AGameplayPlayerStart*> TeamSpawnLocations;
	TArray<AActor*> FlagPlacements; // TODO change actor to new class

	TArray<class AThirdPersonCharacter*> TeamPawns;

	int32 HumanPlayersCount_RedTeam = 0;
	int32 HumanPlayersCount_BlueTeam = 0;

	TArray<class AGameplayAIController*> InGameControllers_AI;
	TArray<class AGamePlayerController*> InGameControllers_Human;

	// Inital setup logic
	void SetupSpawnLocations();
	void SetupPlayableCharacters();
	//

	class AGameplayGameState* GameplayState;

	// BEGIN Match logic
	void InitialMatchStateSetup();
	void ResetPawnsForNewRound();

	void MatchPhaseStart_Warmup();
	void MatchPhaseStart_Gameplay();
	void MatchPhaseStart_RoundEnd();
	void MatchPhaseEnd_Warmup(const struct FMatchParameters& MatchParameters, const struct FMatchData& CurrentMatchData);
	void MatchPhaseEnd_Gameplay(const struct FMatchParameters& MatchParameters, const struct FMatchData& CurrentMatchData);
	void MatchPhaseEnd_RoundEnd(const struct FMatchParameters& MatchParameters, const struct FMatchData& CurrentMatchData);

	void StopCurrentMatchTimer();
	FTimerHandle MatchTimerHandle;
	// END Match logic

	static const FString NewPlayerOptionsNameKey; // When new PlayerController gets created, set its name from option parameter with that key name on AMainGameMode::Login() 

private:
	int32 GetNextSpawnLocationIndex(int32 StartingIndex, ETeamType TeamType);
	int32 GetNextPlayerControllerIndex(int32 StartingIndex, ETeamType TeamType);
};
