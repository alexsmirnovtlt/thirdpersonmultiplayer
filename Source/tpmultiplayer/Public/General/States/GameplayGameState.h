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

	FMatchData() : FMatchData(0, 0, 0, 0.f) {};
	FMatchData(uint8 PlayersAlive_FirstTeam, uint8 PlayersAlive_SecondTeam, uint8 MaxRoundsNum, float ServerTime)
		: FirstTeam_PlayersAlive(PlayersAlive_FirstTeam), SecondTeam_PlayersAlive(PlayersAlive_SecondTeam), MaxRounds(MaxRoundsNum), MatchStartServerTime(ServerTime)
	{
		MatchState = EMatchState::Warmup;
		SpecialMessage = EInGameSpecialMessage::Nothing;
		FirstTeam_MatchesWon = 0;
		SecondTeam_MatchesWon = 0;
	}

public:
	EMatchState MatchState;
	EInGameSpecialMessage SpecialMessage;

	uint8 FirstTeam_PlayersAlive;
	uint8 FirstTeam_MatchesWon;

	uint8 SecondTeam_PlayersAlive;
	uint8 SecondTeam_MatchesWon;

	float MatchStartServerTime;

	uint8 MaxRounds;

	const FString GetRoundProgressString() const
	{
		return FString::Printf(TEXT("%d/%d"), FirstTeam_MatchesWon + SecondTeam_MatchesWon + 1, MaxRounds);
	}

	const FString DebugToString() const
	{
		return FString::Printf(TEXT("MatchState:%d - Round:%d/%d, FirstTeam(Alive/MatchesWon): %d/%d	SecondTeam(Alive/MatchesWon):%d/%d	 started at %f"), MatchState, FirstTeam_MatchesWon + SecondTeam_MatchesWon + 1, MaxRounds, FirstTeam_PlayersAlive, FirstTeam_MatchesWon, SecondTeam_PlayersAlive, SecondTeam_MatchesWon, MatchStartServerTime);
	}
};


USTRUCT(Blueprintable)
struct FMatchParameters
{
	GENERATED_BODY()
	// Time periods are like csgo, WarmupPeriodSec when players spawned but cant move,
	// EndgamePeriodSec is time after one of the teams won the round, before respawn and Warmup again
	FMatchParameters()
	{
		WarmupPeriodSec = 3; // TODO Change to 5
		MatchPeriodSec = 4; // TODO Change to 150
		EndRoundPeriodSec = 2; // TODO Change to 5
		MaxGameRounds = 3; // TODO Change to 5
		MaxGameRoundsToWin = 2; // TODO Change to 3
	}

public:
	UPROPERTY(EditAnywhere)
	int32 WarmupPeriodSec;
	UPROPERTY(EditAnywhere)
	int32 MatchPeriodSec;
	UPROPERTY(EditAnywhere)
	int32 EndRoundPeriodSec;
	UPROPERTY(EditAnywhere)
	int32 MaxGameRounds;
	UPROPERTY(EditAnywhere)
	int32 MaxGameRoundsToWin;
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

	const FMatchData& GetCurrentMatchData() { return CurrentMatchData; };
	const FMatchParameters& GetMatchParameters() { return MatchParameters; };

	uint8 GetNumPlayers_Redteam() { return CurrentPlayers_RedTeam; };
	uint8 GetNumPlayers_Blueteam() { return CurrentPlayers_BlueTeam; };

	FVector& GetSpectatorInitialSpawnLocation() { return SpectatorInitialSpawnLocation; };
	FRotator& GetSpectatorInitialSpawnRotation() { return SpectatorInitialSpawnRotation; };

	FOnMatchDataChangedDelegate OnMatchDataChangedEvent;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Time Periods And Score Setup")
	FMatchParameters MatchParameters;

	void InitialMatchStateSetup();
	void ProceedToNextMatchState();

	UFUNCTION()
	void OnMatchTimerEnded();
	FTimerHandle MatchTimerHandle;

	UPROPERTY(ReplicatedUsing = OnMatchStateChanged)
	FMatchData CurrentMatchData;
	UFUNCTION()
	void OnMatchStateChanged();

	// Public replicated properties that can be accessed from anywhere
public:
	UPROPERTY(Replicated)
	uint8 CurrentPlayers_RedTeam;
	UPROPERTY(Replicated)
	uint8 CurrentPlayers_BlueTeam;

	UPROPERTY(Replicated)
	FVector SpectatorInitialSpawnLocation; // New players are created without any Pawn to control so when they start to spectate without ever joining the match thay need a location insted of spawning at FVector::ZeroVector
	UPROPERTY(Replicated)
	FRotator SpectatorInitialSpawnRotation; // Same as SpectatorInitialSpawnLocation above
};
