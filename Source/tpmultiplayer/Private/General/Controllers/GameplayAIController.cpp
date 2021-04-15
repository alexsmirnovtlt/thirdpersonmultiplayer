// Aleksandr Smirnov 2021


#include "General/Controllers/GameplayAIController.h"

#include "General/Pawns/ThirdPersonCharacter.h"
#include "General/States/GameplayGameState.h"

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
	if (GameState) GameState->OnMatchDataChangedEvent.RemoveDynamic(this, &AGameplayAIController::OnMatchStateChanged);
}

void AGameplayAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetPawn() || !PossessedCharacter) return;

	// TMP DEBUG
	if (CurrentMatchState == EMatchState::Warmup)
	{
		PossessedCharacter->StopJumping();

		// TODO debug rotate
	}
	else if (CurrentMatchState == EMatchState::Gameplay)
	{
		PossessedCharacter->MoveForward(DEBUG_MovementsSpeed);
	}
	else if (CurrentMatchState == EMatchState::RoundEnd)
	{
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

	GameState->OnMatchDataChangedEvent.AddDynamic(this, &AGameplayAIController::OnMatchStateChanged);
	OnMatchStateChanged();
}

void AGameplayAIController::OnUnPossess()
{
	if (GameState) GameState->OnMatchDataChangedEvent.RemoveDynamic(this, &AGameplayAIController::OnMatchStateChanged);

	Super::OnUnPossess();
}

void AGameplayAIController::OnMatchStateChanged()
{
	if (!GameState) return;

	auto& MatchData = GameState->GetCurrentMatchData();
	CurrentMatchState = MatchData.MatchState;
}