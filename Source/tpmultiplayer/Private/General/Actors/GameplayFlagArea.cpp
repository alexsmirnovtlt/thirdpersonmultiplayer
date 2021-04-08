// Aleksandr Smirnov 2021


#include "General/Actors/GameplayFlagArea.h"

#include "General/Pawns/ThirdPersonCharacter.h"
#include "General/States/GameplayGameState.h"
#include "General/GameModes/MainGameMode.h"

AGameplayFlagArea::AGameplayFlagArea()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AGameplayFlagArea::InitialSetup(class AMainGameMode* GameModePtr, class AGameplayGameState* GameStatePtr)
{
	MainGameMode = GameModePtr;
	GameState = GameStatePtr;
}

void AGameplayFlagArea::ResetFlagState()
{
	OnFlagStateReset(); // call to BP
}

void AGameplayFlagArea::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	auto Player = Cast<AThirdPersonCharacter>(OtherActor);
	if (!Player) return;


}

void AGameplayFlagArea::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	auto Player = Cast<AThirdPersonCharacter>(OtherActor);
	if (!Player) return;


}

void AGameplayFlagArea::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


}