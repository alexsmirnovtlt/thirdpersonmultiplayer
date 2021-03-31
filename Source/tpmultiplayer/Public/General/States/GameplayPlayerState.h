// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayPlayerState.generated.h"

UENUM(BlueprintType)
enum class ETeamType : uint8 {
	Spectator = 0 UMETA(DisplayName = "Spectator"),
	RedTeam = 1 UMETA(DisplayName = "Red Team"),
	BlueTeam = 2 UMETA(DisplayName = "Blue Team")
};

/**
 * 
 */
UCLASS()
class TPMULTIPLAYER_API AGameplayPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	ETeamType TeamType;
};
