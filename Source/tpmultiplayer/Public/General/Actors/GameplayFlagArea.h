// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayFlagArea.generated.h"

UCLASS()
class TPMULTIPLAYER_API AGameplayFlagArea : public AActor
{
	GENERATED_BODY()
	
public:	
	AGameplayFlagArea();
protected:
	virtual void BeginPlay() override;
};
