// Aleksandr Smirnov 2021


#include "General/States/GameplayGameState.h"

#include "Net/UnrealNetwork.h"

#include "General/GameModes/MainGameMode.h"

void AGameplayGameState::BeginPlay()
{
	Super::BeginPlay();

	// Saving initial spawn location and rotation for new Player Controllers, because they spawn without attached Pawn at FVector::ZeroVector
	if (HasAuthority() && GetWorld())
	{
		if (auto SpawnActor = GetWorld()->GetAuthGameMode<AMainGameMode>()->GetInitialSpawnLocationActor())
		{
			SpectatorInitialSpawnLocation = SpawnActor->GetActorLocation();
			SpectatorInitialSpawnRotation = SpawnActor->GetActorRotation();
		}
	}
}

void AGameplayGameState::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGameplayGameState, SpectatorInitialSpawnLocation);
	DOREPLIFETIME(AGameplayGameState, SpectatorInitialSpawnRotation);
}