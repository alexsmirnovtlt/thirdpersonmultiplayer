// Aleksandr Smirnov 2021


#include "General/States/GameplayPlayerState.h"

#include "Net/UnrealNetwork.h"

void AGameplayPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority()) CurrentHealth = StartingHealth;
}

void AGameplayPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGameplayPlayerState, TeamType);
	DOREPLIFETIME(AGameplayPlayerState, CurrentHealth);
}