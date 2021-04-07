// Aleksandr Smirnov 2021


#include "General/Actors/GameplayFlagArea.h"

#include "General/GameModes/MainGameMode.h"

AGameplayFlagArea::AGameplayFlagArea()
{
	PrimaryActorTick.bCanEverTick = false;
	bNetLoadOnClient = false;
}

void AGameplayFlagArea::BeginPlay()
{
	Super::BeginPlay();
	
}