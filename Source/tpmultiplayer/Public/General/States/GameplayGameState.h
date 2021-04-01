// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayGameState.generated.h"

UENUM(BlueprintType)
enum class ETeamType : uint8 {
	Spectator = 0,
	RedTeam = 1,
	BlueTeam = 2
};

UENUM()
enum class EMatchState : uint8 {
	Warmup = 0,
	Gameplay = 1,
	RoundEnd = 2
};

UENUM()
enum class EInGameSpecialMessage : uint8 {
	Nothing = 0,
	RedTeamWonLastRound = 1,
	BlueTeamWonLastRound = 2,
	RedTeamWonLastGame = 3,
	BlueTeamWonLastGame = 4
};

USTRUCT()
struct FMatchData
{
	GENERATED_BODY()

	FMatchData() : FMatchData(0, 0, 0.f) {};
	FMatchData(uint8 PlayersAlive_FirstTeam, uint8 PlayersAlive_SecondTeam, float ServerTime)
		: FirstTeam_PlayersAlive(PlayersAlive_FirstTeam), SecondTeam_PlayersAlive(PlayersAlive_SecondTeam), MatchStartServerTime(ServerTime)
	{
		MatchState = EMatchState::Warmup;
		FirstTeam_MatchesWon = 0;
		SecondTeam_MatchesWon = 0;
	}

public:
	EMatchState MatchState;

	uint8 FirstTeam_PlayersAlive;
	uint8 FirstTeam_MatchesWon;

	uint8 SecondTeam_PlayersAlive;
	uint8 SecondTeam_MatchesWon;

	float MatchStartServerTime;

	const FString DebugToString()
	{
		return FString::Printf(TEXT("%d - %d/%d - %d/%d -- started at %d	PlayersAlive/MatchesWon"), MatchState, FirstTeam_PlayersAlive, FirstTeam_MatchesWon, SecondTeam_PlayersAlive, SecondTeam_MatchesWon, MatchStartServerTime);
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchDataChangedDelegate);

/**
 * 
 */
UCLASS()
class TPMULTIPLAYER_API AGameplayGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AGameplayGameState();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	void AddPlayerToAMatch(class APlayerController* PlayerController);
	void RemovePlayerFromAMatch(class APlayerController* PlayerController);

	const FMatchData& GetCurrentMatchData() { return CurrentMatchData; };

	uint8 GetNumPlayers_Redteam() { return CurrentPlayers_RedTeam; };
	uint8 GetNumPlayers_Blueteam() { return CurrentPlayers_BlueTeam; };

	FVector& GetSpectatorInitialSpawnLocation() { return SpectatorInitialSpawnLocation; };
	FRotator& GetSpectatorInitialSpawnRotation() { return SpectatorInitialSpawnRotation; };

	FOnMatchDataChangedDelegate OnMatchDataChangedEvent;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Classes")
	TSubclassOf<class AThirdPersonCharacter> GameplayPawnClass_RedTeam;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Classes")
	TSubclassOf<class AThirdPersonCharacter> GameplayPawnClass_BlueTeam;

	UPROPERTY(EditDefaultsOnly, Category = "Time Periods And Score Setup")
	int32 WarmupPeriodSec; // Time periods are like csgo, WarmupPeriodSec when players spawned but cant move, EndgamePeriodSec is time after one of the teams won the round, before respawn and Warmup again
	UPROPERTY(EditDefaultsOnly, Category = "Time Periods And Score Setup")
	int32 MatchPeriodSec;
	UPROPERTY(EditDefaultsOnly, Category = "Time Periods And Score Setup")
	int32 EndRoundPeriodSec;
	UPROPERTY(EditDefaultsOnly, Category = "Time Periods And Score Setup")
	int32 MaxGameRounds;
	UPROPERTY(EditDefaultsOnly, Category = "Time Periods And Score Setup")
	int32 MaxGameRoundsToWin;

	void SetupSpawnLocations(); // Getting all PlayerStarts from level to have locations for players to spawn
	void SetupPlayableCharacters(); // Happend once on BeginPlay() - occupies all available spawn points with AI controlled characters so new players can possess them or just watch
	void InitialMatchStateSetup();

	void ProceedToNextMatchState();

	UFUNCTION()
	void OnMatchTimerEnded();
	FTimerHandle MatchTimerHandle;

	AActor* SpectatorSpawn;
	TArray<AActor*> TeamSpawns_Red;
	TArray<AActor*> TeamSpawns_Blue;

	UPROPERTY(ReplicatedUsing = OnMatchStateChanged)
	FMatchData CurrentMatchData;
	UFUNCTION()
	void OnMatchStateChanged();

	UPROPERTY(Replicated)
	uint8 CurrentPlayers_RedTeam;
	UPROPERTY(Replicated)
	uint8 CurrentPlayers_BlueTeam;

	UPROPERTY(Replicated)
	FVector SpectatorInitialSpawnLocation; // New players are created without any Pawn to control so when they start to spectate without ever joining the match thay need a location insted of spawning at FVector::ZeroVector
	UPROPERTY(Replicated)
	FRotator SpectatorInitialSpawnRotation; // Same as SpectatorInitialSpawnLocation above
};
