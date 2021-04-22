// Aleksandr Smirnov 2021


#include "General/Pawns/GameplaySpectatorPawn.h"
#include "General/Controllers/GamePlayerController.h"

void AGameplaySpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	 // Wont be calling Super because the only thing it does is setups default input keys

	PlayerInputComponent->BindAxis(AGamePlayerController::HorizontalAxisBindingName, this, &AGameplaySpectatorPawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis(AGamePlayerController::VerticalAxisBindingName, this, &AGameplaySpectatorPawn::AddControllerPitchInput_Reversed);
	PlayerInputComponent->BindAxis(AGamePlayerController::MoveForwardAxisBindingName, this, &AGameplaySpectatorPawn::MoveForward);
	PlayerInputComponent->BindAxis(AGamePlayerController::MoveRightAxisBindingName, this, &AGameplaySpectatorPawn::MoveRight);
	PlayerInputComponent->BindAxis(AGamePlayerController::ShootBindingName, this, &AGameplaySpectatorPawn::MoveUp_World);
	PlayerInputComponent->BindAxis(AGamePlayerController::AimBindingName, this, &AGameplaySpectatorPawn::MoveDown_World);
}