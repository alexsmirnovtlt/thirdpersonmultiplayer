// Aleksandr Smirnov 2021


#include "General/ActorComponents/TPCMovementComponent.h"

#include "AbilitySystemComponent.h"

#include "General/Pawns/ThirdPersonCharacter.h"


UTPCMovementComponent::UTPCMovementComponent()
{
	SprintingMaxSpeed = 600.f;
}

float UTPCMovementComponent::GetMaxSpeed() const
{
	auto Owner = Cast<AThirdPersonCharacter>(GetOwner());
	if (!Owner) return 0.f;

	if (RequestToStartSprinting || Owner->GetAbilitySystemComponent()->HasAnyMatchingGameplayTags(SprintingTags))
		return SprintingMaxSpeed;
	else if (Owner->GetAbilitySystemComponent()->HasAnyMatchingGameplayTags(StationaryTags))
		return 0.f;

	return MaxWalkSpeed;
}

void UTPCMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	RequestToStartSprinting = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void UTPCMovementComponent::FGDSavedMove::Clear()
{
	Super::Clear();
	SavedRequestToStartSprinting = 0;
}

uint8 UTPCMovementComponent::FGDSavedMove::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (SavedRequestToStartSprinting) Result |= FLAG_Custom_0;
	return Result;
}

bool UTPCMovementComponent::FGDSavedMove::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	if (SavedRequestToStartSprinting != ((FGDSavedMove*)&NewMove)->SavedRequestToStartSprinting) return false;
	else return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void UTPCMovementComponent::FGDSavedMove::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	if(UTPCMovementComponent* CharacterMovement = Cast<UTPCMovementComponent>(Character->GetCharacterMovement()))
		SavedRequestToStartSprinting = CharacterMovement->RequestToStartSprinting;
}