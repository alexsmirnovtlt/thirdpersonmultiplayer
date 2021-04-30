// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayStructs.generated.h"

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	None = 0,
	AbilityCancel = 1,
	AbilityConfirm = 2,
	Shoot = 3,
	Aim = 4,
	Sprint = 5,
	Reload = 6
};

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
	BlueTeamWonLastGame = 4,
	AreaCaptureInProgress = 5
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
		CurrentRound = 1;
		RedTeamHasFlag = true;
		AreaWasCaptured = false;
		VIPWasKilled = false;
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
	uint8 MaxRounds;

	UPROPERTY()
	float MatchStartServerTime;

	UPROPERTY()
	uint8 CurrentRound;

	UPROPERTY()
	bool RedTeamHasFlag;

	UPROPERTY()
	bool AreaWasCaptured;

	UPROPERTY()
	bool VIPWasKilled;

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
		MatchPeriodSec = 60; // TODO Change to 150
		EndRoundPeriodSec = 4; // TODO Change to 8
		MaxGameRounds = 3; // TODO Change to 5 (15)
		MaxGameRoundsToWin = 2; // TODO Change to 3 (8)
		FlagDefenseTime = 5; // TODO Change to 20?
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
	UPROPERTY(EditAnywhere)
	int32 FlagDefenseTime;
};

USTRUCT(Blueprintable)
struct FCharacterAnimState
{
	GENERATED_BODY()

	FCharacterAnimState()
	{
		bIsDead = false;
		bIsAiming = false;
		bIsReloading = false;
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDead;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAiming;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReloading;
};

USTRUCT(Blueprintable)
struct FShootData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AActor* Shooter;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AActor* Target;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsValidHit; // Should we visualize ImpactLocation and ImpactNormal
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsClipEmpty;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector_NetQuantize ImpactLocation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector_NetQuantizeNormal ImpactNormal;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ServerTime;
};