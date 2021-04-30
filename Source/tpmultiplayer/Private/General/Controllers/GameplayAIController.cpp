// Aleksandr Smirnov 2021


#include "General/Controllers/GameplayAIController.h"

#include "Delegates/IDelegateInstance.h"
#include "AbilitySystemComponent.h"

#include "General/Pawns/ThirdPersonCharacter.h"
#include "General/States/GameplayGameState.h"
#include "General/GameplayStructs.h"

AGameplayAIController::AGameplayAIController()
{

}

void AGameplayAIController::BeginPlay()
{
	Super::BeginPlay();

	DEBUG_ClockwiseRotation = FMath::SRand() > 0.5f;
	DEBUG_JumpPeriod = FMath::RandRange(0.2f, 0.8f);
	DEBUG_MovementsSpeed = FMath::RandRange(0.3f, 1.f);
	DEBUG_RotationSpeed = FMath::RandRange(0.2f, 1.3f);
}

void AGameplayAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AGameplayAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetPawn() || !PossessedCharacter) return;

	// TMP DEBUG
	ActionTime += DeltaTime;

	bool bActionTime = false;
	
	if (ActionTime > 1.f)
	{
		ActionTime = 0.f;
		bActionTime = true;
	}

	if (CurrentMatchState == EMatchState::Warmup)
	{
		if (bActionTime)
		{
			PossessedCharacter->StopJumping();
			FGameplayTagContainer tagcontainer;
			tagcontainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Reload")));
			//PossessedCharacter->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(tagcontainer);
		}
		
		PossessedCharacter->MoveForward(DEBUG_MovementsSpeed);
	}
	else if (CurrentMatchState == EMatchState::Gameplay)
	{
		if (bActionTime)
		{
			if (!PossessedCharacter->IsInAimingAnimation())
			{
				FGameplayTagContainer tagcontainer;
				tagcontainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Aim")));
				//PossessedCharacter->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(tagcontainer);
				//PossessedCharacter->GetAbilitySystemComponent()->AbilityLocalInputPressed((int32)EAbilityInputID::Aim);
			}
		}

		PossessedCharacter->MoveForward(DEBUG_MovementsSpeed);
	}
	else if (CurrentMatchState == EMatchState::RoundEnd)
	{
		if (bActionTime)
		{
			if (PossessedCharacter->IsInAimingAnimation())
			{
				FGameplayTagContainer tagcontainer;
				tagcontainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Aim")));
				//PossessedCharacter->GetAbilitySystemComponent()->CancelAbilities(&tagcontainer);
				//PossessedCharacter->GetAbilitySystemComponent()->AbilityLocalInputReleased((int32)EAbilityInputID::Aim);
			}
		}

		DEBUG_DeltaTimePassed += DeltaTime;
		if (DEBUG_DeltaTimePassed > DEBUG_JumpPeriod)
		{
			DEBUG_DeltaTimePassed = 0;
			PossessedCharacter->Jump();
		}
	}
	// TMP DEBUG
}

void AGameplayAIController::OnPossess(class APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PossessedCharacter = Cast<AThirdPersonCharacter>(InPawn);
	if (!PossessedCharacter || !GameState) return;

	OnMatchStateChanged();
	MatchStateChangedDelegateHandle = GameState->OnMatchDataChanged().AddUObject(this, &AGameplayAIController::OnMatchStateChanged);
}

void AGameplayAIController::OnUnPossess()
{
	Super::OnUnPossess();

	GameState->OnMatchDataChanged().Remove(MatchStateChangedDelegateHandle);
}

void AGameplayAIController::OnMatchStateChanged()
{
	if (!GameState) return;

	auto& MatchData = GameState->GetCurrentMatchData();
	CurrentMatchState = MatchData.MatchState;
}