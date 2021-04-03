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
		CurrentRound = 0;
		RedTeamHasFlag = true;
	}

public:
	UPROPERTY()
	EMatchState MatchState;
	UPROPERTY()
	EInGameSpecialMessage SpecialMessage;
	
	UPROPERTY()
	uint8 FirstTeam_PlayersAlive;
	UPROPERTY()
	uint8 FirstTeam_MatchesWon;

	UPROPERTY()
	uint8 SecondTeam_PlayersAlive;
	UPROPERTY()
	uint8 SecondTeam_MatchesWon;

	UPROPERTY()
	float MatchStartServerTime;

	UPROPERTY()
	uint8 MaxRounds;

	UPROPERTY()
	uint8 CurrentRound;

	UPROPERTY()
	bool RedTeamHasFlag;

	const FString GetRoundProgressString() const
	{
		return FString::Printf(TEXT("%d/%d"), CurrentRound, MaxRounds);
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
		WarmupPeriodSec = 4; // TODO Change to 8
		MatchPeriodSec = 4; // TODO Change to 150
		EndRoundPeriodSec = 4; // TODO Change to 8
		MaxGameRounds = 3; // TODO Change to 5 (15)
		MaxGameRoundsToWin = 2; // TODO Change to 3 (8)
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

	friend class AMainGameMode; // only AuthGameMode is able to change CurrentMatchData that gets replicated to everyone

public:
	const FMatchData& GetCurrentMatchData() { return CurrentMatchData; };
	const FMatchParameters& GetMatchParameters() { return MatchParameters; };

	uint8 GetNumPlayers_Redteam() { return CurrentPlayers_RedTeam; };
	uint8 GetNumPlayers_Blueteam() { return CurrentPlayers_BlueTeam; };

	FVector& GetSpectatorInitialSpawnLocation() { return SpectatorInitialSpawnLocation; };
	FRotator& GetSpectatorInitialSpawnRotation() { return SpectatorInitialSpawnRotation; };

	FOnMatchDataChangedDelegate OnMatchDataChangedEvent;

protected:

	UPROPERTY(ReplicatedUsing = OnRep_MatchStateChanged)
	FMatchData CurrentMatchData;

	UPROPERTY(EditDefaultsOnly, Category = "Time Periods And Score Setup")
	FMatchParameters MatchParameters;

	UFUNCTION()
	void OnRep_MatchStateChanged();

	UPROPERTY(Replicated)
	uint8 CurrentPlayers_RedTeam;
	UPROPERTY(Replicated)
	uint8 CurrentPlayers_BlueTeam;

	UPROPERTY(Replicated)
	FVector SpectatorInitialSpawnLocation; // New players are created without any Pawn to control so when they start to spectate without ever joining the match thay need a location insted of spawning at FVector::ZeroVector
	UPROPERTY(Replicated)
	FRotator SpectatorInitialSpawnRotation; // Same as SpectatorInitialSpawnLocation above
};
