// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayGameState.generated.h"

/**
 * 
 */
UCLASS()
class TPMULTIPLAYER_API AGameplayGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	FVector& GetSpectatorInitialSpawnLocation() { return SpectatorInitialSpawnLocation; };
	FRotator& GetSpectatorInitialSpawnRotation() { return SpectatorInitialSpawnRotation; };

protected:
	UPROPERTY(Replicated)
	FVector SpectatorInitialSpawnLocation;
	UPROPERTY(Replicated)
	FRotator SpectatorInitialSpawnRotation;
};
