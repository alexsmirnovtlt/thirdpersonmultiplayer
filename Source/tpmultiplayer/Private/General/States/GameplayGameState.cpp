// Aleksandr Smirnov 2021


#include "General/States/GameplayGameState.h"

#include "Net/UnrealNetwork.h"


AGameplayGameState::AGameplayGameState()
{
	NetUpdateFrequency = 0;
	MatchParameters = FMatchParameters();
}

void AGameplayGameState::BeginPlay()
{
	Super::BeginPlay();
}

void AGameplayGameState::OnRep_MatchStateChanged()
{
	// Gets called on server and clients every time MatchData was updated so event subscribers can get updated data

	// TODO Remove debug info
	//if (HasAuthority()) { UE_LOG(LogTemp, Warning, TEXT("SERVER AGameplayGameState::OnMatchStateChanged: %s"), *CurrentMatchData.DebugToString()); }
	//else { UE_LOG(LogTemp, Warning, TEXT("CLIENT AGameplayGameState::OnMatchStateChanged: %s"), *CurrentMatchData.DebugToString()); }
	//

	OnMatchDataChangedEvent.Broadcast();
}

void AGameplayGameState::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGameplayGameState, CurrentMatchData);

	DOREPLIFETIME_CONDITION(AGameplayGameState, CurrentPlayers_RedTeam, COND_InitialOnly); 
	DOREPLIFETIME_CONDITION(AGameplayGameState, CurrentPlayers_BlueTeam, COND_InitialOnly);
}