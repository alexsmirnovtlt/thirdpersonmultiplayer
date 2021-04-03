// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayPlayerState.generated.h"

enum class ETeamType : uint8;

/**
 * 
 */
UCLASS()
class TPMULTIPLAYER_API AGameplayPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(Replicated)
	ETeamType TeamType;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	float StartingHealth = 100;

	UPROPERTY(Replicated)
	float CurrentHealth;
};
