// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DefaultPawnAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class TPMULTIPLAYER_API UDefaultPawnAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadOnly, Category = "AttributeSet", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UDefaultPawnAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "AttributeSet")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UDefaultPawnAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "AttributeSet", ReplicatedUsing = OnRep_AmmoCount)
	FGameplayAttributeData AmmoCount;
	ATTRIBUTE_ACCESSORS(UDefaultPawnAttributeSet, AmmoCount)

	UPROPERTY(BlueprintReadOnly, Category = "AttributeSet")
	FGameplayAttributeData PistolClipSize;
	ATTRIBUTE_ACCESSORS(UDefaultPawnAttributeSet, PistolClipSize)

	UPROPERTY(BlueprintReadOnly, Category = "AttributeSet")
	FGameplayAttributeData WeaponDamage;
	ATTRIBUTE_ACCESSORS(UDefaultPawnAttributeSet, WeaponDamage)

protected:

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_AmmoCount(const FGameplayAttributeData& OldValue);
};