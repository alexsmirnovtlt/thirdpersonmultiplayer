// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "General/GameplayStructs.h"
#include "GameFramework/GameStateBase.h"

#include "GameplayGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchDataChangedDelegate);

/**
 * Main GameState that used for replicating Match State related parameters
 */
UCLASS()
class TPMULTIPLAYER_API AGameplayGameState : public AGameStateBase
{
	GENERATED_BODY()

	friend class AMainGameMode; // only AuthGameMode is able to change CurrentMatchData that gets replicated to everyone

public:
	AGameplayGameState();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	const FMatchData& GetCurrentMatchData() { return CurrentMatchData; };
	const FMatchParameters& GetMatchParameters() { return MatchParameters; };

	const uint8 GetNumPlayers_Redteam() { return CurrentPlayers_RedTeam; };
	const uint8 GetNumPlayers_Blueteam() { return CurrentPlayers_BlueTeam; };

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
};
