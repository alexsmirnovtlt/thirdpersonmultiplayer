// Aleksandr Smirnov 2021


#include "General/States/GameplayGameState.h"

#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"


#include "General/Controllers/GamePlayerController.h"

AGameplayGameState::AGameplayGameState()
{
	MatchParameters = FMatchParameters();
}

void AGameplayGameState::BeginPlay()
{
	Super::BeginPlay();

	InitialMatchStateSetup();
}

// BEGIN Match related logic

void AGameplayGameState::InitialMatchStateSetup()
{
	CurrentMatchData = FMatchData(CurrentPlayers_RedTeam, CurrentPlayers_BlueTeam, MatchParameters.MaxGameRounds, GetServerWorldTimeSeconds());

	// DEBUG
	if (HasAuthority())OnMatchStateChanged();
	GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this, &AGameplayGameState::OnMatchTimerEnded, MatchParameters.WarmupPeriodSec, false);
	//
}

void AGameplayGameState::OnMatchStateChanged()
{
	// TODO Remove debug info
	if (HasAuthority()) { UE_LOG(LogTemp, Warning, TEXT("SERVER AGameplayGameState::OnMatchStateChanged: %s"), *CurrentMatchData.DebugToString()); }
	else { UE_LOG(LogTemp, Warning, TEXT("CLIENT AGameplayGameState::OnMatchStateChanged: %s"), *CurrentMatchData.DebugToString()); }
	//

	OnMatchDataChangedEvent.Broadcast();
}

void AGameplayGameState::ProceedToNextMatchState()
{

}

void AGameplayGameState::OnMatchTimerEnded()
{
	// START DEBUG
	int32 NextMatchState = (int32)CurrentMatchData.MatchState + 1;
	if (NextMatchState > 2) NextMatchState = 0;

	CurrentMatchData.MatchState = (EMatchState) NextMatchState;
	
	float TimerTime;

	if (CurrentMatchData.MatchState == EMatchState::Warmup)
	{
		TimerTime = MatchParameters.WarmupPeriodSec;
	}
	else if (CurrentMatchData.MatchState == EMatchState::Gameplay)
	{
		TimerTime = MatchParameters.MatchPeriodSec;
	}
	else if (CurrentMatchData.MatchState == EMatchState::RoundEnd)
	{
		TimerTime = MatchParameters.EndRoundPeriodSec;
	}

	CurrentMatchData.MatchStartServerTime = GetServerWorldTimeSeconds();

	//
	if(HasAuthority()) OnMatchStateChanged();

	GetWorld()->GetTimerManager().SetTimer(MatchTimerHandle, this, &AGameplayGameState::OnMatchTimerEnded, TimerTime, false);

	// END DEBUG
}

// END Match related logic

void AGameplayGameState::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGameplayGameState, CurrentMatchData);

	DOREPLIFETIME_CONDITION(AGameplayGameState, SpectatorInitialSpawnLocation, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(AGameplayGameState, SpectatorInitialSpawnRotation, COND_InitialOnly);

	DOREPLIFETIME_CONDITION(AGameplayGameState, CurrentPlayers_RedTeam, COND_InitialOnly); 
	DOREPLIFETIME_CONDITION(AGameplayGameState, CurrentPlayers_BlueTeam, COND_InitialOnly);
}