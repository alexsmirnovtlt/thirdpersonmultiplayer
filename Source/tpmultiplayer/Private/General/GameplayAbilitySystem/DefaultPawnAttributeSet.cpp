// Aleksandr Smirnov 2021


#include "General/GameplayAbilitySystem/DefaultPawnAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

#include "General/Pawns/ThirdPersonCharacter.h"

void UDefaultPawnAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	AThirdPersonCharacter* TargetPawn = nullptr;

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		auto TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		TargetPawn = Cast<AThirdPersonCharacter>(TargetActor);
	}

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// TODO Clamp Health, notify BP
		// TODO Handle Health <= 0
	}
	else if (Data.EvaluatedData.Attribute == GetMovementSpeedAttribute())
	{
		UE_LOG(LogTemp, Warning, TEXT("PostGameplayEffectExecute"));
		if(TargetPawn) TargetPawn->OnMaxSpeedChanged(GetMovementSpeed());
	}
}

void UDefaultPawnAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UDefaultPawnAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDefaultPawnAttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always);
}

void UDefaultPawnAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDefaultPawnAttributeSet, Health, OldValue);
}

void UDefaultPawnAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDefaultPawnAttributeSet, MovementSpeed, OldValue);
}