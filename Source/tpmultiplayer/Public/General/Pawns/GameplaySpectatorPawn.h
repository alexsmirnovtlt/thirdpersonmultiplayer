// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "GameplaySpectatorPawn.generated.h"

/**
 * Default spectator Pawn for a gameplay map
 */
UCLASS()
class TPMULTIPLAYER_API AGameplaySpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()
public:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
protected:
	void MoveDown_World(float value) { MoveUp_World(-value); };
};
