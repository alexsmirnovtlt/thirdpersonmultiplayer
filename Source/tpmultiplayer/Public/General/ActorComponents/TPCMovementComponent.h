// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "TPCMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class TPMULTIPLAYER_API UTPCMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
	class FGDSavedMove : public FSavedMove_Character
	{
	public:

		typedef FSavedMove_Character Super;

		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;

		uint8 SavedRequestToStartSprinting : 1;
	};

	class FNetworkPredictionDataCharacter_Client : public FNetworkPredictionData_Client_Character
	{
	public:

		FNetworkPredictionDataCharacter_Client(const UCharacterMovementComponent& ClientMovement) : FNetworkPredictionData_Client_Character(ClientMovement) {};
		virtual FSavedMovePtr AllocateNewMove() override { return FSavedMovePtr(new FGDSavedMove()); };
	};

public:
	UTPCMovementComponent();

	virtual float GetMaxSpeed() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

public:
	UFUNCTION(BlueprintCallable, Category = "Third Person Character Movement")
	void StartSprinting() { RequestToStartSprinting = true; };
	UFUNCTION(BlueprintCallable, Category = "Third Person Character Movement")
	void StopSprinting() { RequestToStartSprinting = false; };

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Third Person Character Movement")
	float SprintingMaxSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "Third Person Character Movement")
	FGameplayTagContainer SprintingTags;
	UPROPERTY(EditDefaultsOnly, Category = "Third Person Character Movement")
	FGameplayTagContainer StationaryTags;

	uint8 RequestToStartSprinting : 1;
};
