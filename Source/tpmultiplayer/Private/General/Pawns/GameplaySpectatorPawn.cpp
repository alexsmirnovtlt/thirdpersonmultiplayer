// Aleksandr Smirnov 2021


#include "General/Pawns/GameplaySpectatorPawn.h"

#include "General/Controllers/GamePlayerController.h"
#include "General/HUD/GameplayHUD.h"


void AGameplaySpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	 // Wont be calling Super because the only thing it does is setups default input keys

	PlayerInputComponent->BindAxis(AGamePlayerController::HorizontalAxisBindingName, this, &AGameplaySpectatorPawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis(AGamePlayerController::VerticalAxisBindingName, this, &AGameplaySpectatorPawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis(AGamePlayerController::MoveForwardAxisBindingName, this, &AGameplaySpectatorPawn::MoveForward);
	PlayerInputComponent->BindAxis(AGamePlayerController::MoveRightAxisBindingName, this, &AGameplaySpectatorPawn::MoveRight);
	PlayerInputComponent->BindAxis(AGamePlayerController::PrimaryActionAxisBindingName, this, &AGameplaySpectatorPawn::MoveUp_World);
	PlayerInputComponent->BindAxis(AGamePlayerController::SecondaryActionAxisBindingName, this, &AGameplaySpectatorPawn::MoveDown_World);

	PlayerInputComponent->BindAction(AGamePlayerController::MenuActionBindingName, EInputEvent::IE_Pressed, this, &AGameplaySpectatorPawn::MenuActionInput);
}

void AGameplaySpectatorPawn::MenuActionInput()
{
	if (!IsValid(GameHUD) && Controller) // Cant able to cache GameHUD at BeginPlay() so trying to do it here
		if (auto PC = Cast<AGamePlayerController>(Controller)) GameHUD = PC->GetGameplayHUD();

	if(IsValid(GameHUD)) GameHUD->MainMenu_Toggle();	
}