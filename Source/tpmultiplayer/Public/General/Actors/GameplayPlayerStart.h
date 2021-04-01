// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"

#include "General/States/GameplayGameState.h"

#include "GameplayPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class TPMULTIPLAYER_API AGameplayPlayerStart : public APlayerStart
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Team Type")
	ETeamType TeamType;
};
