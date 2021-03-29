// Aleksandr Smirnov 2021


#include "General/Controllers/GamePlayerController.h"

#include "GameFramework/SpectatorPawn.h"

#include "General/MultiplayerGameInstance.h"
#include "General/HUD/GameplayHUD.h"

void AGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// TODO Menu cannot be closed now without clicking on a button - change
	// TODO Client cant bind input on join

	// Showing mouse cursor because HUD will spawn a menu
	ChangeInputMode(true);
}

void AGamePlayerController::EndPlay(EEndPlayReason::Type Type)
{
	Super::EndPlay(Type);

	
}

void AGamePlayerController::JoinGameAsPlayer()
{
	GetGameplayHUD()->MainMenu_Hide();
	ChangeInputMode(false);
}

void AGamePlayerController::JoinGameAsSpectator()
{
	GetGameplayHUD()->MainMenu_Hide();
	ChangeInputMode(false);
	Server_PlayerWantsToSpectate();
}

void AGamePlayerController::ReturnToLobby()
{
	auto GameInstance = CastChecked<UMultiplayerGameInstance>(GetWorld()->GetGameInstance());
	FString& LobbyMapName = GameInstance->GetLobbyMapName();

	if (HasAuthority())
	{
		if (GetWorld()->IsPlayInEditor())
		{
			// Otherwise GEngine->Browse() crashes when PIE
			GetWorld()->ServerTravel(LobbyMapName, true);
		}
		else
		{
			auto WorldContext = GetGameInstance()->GetWorldContext();
			FURL URL = FURL(*LobbyMapName);
			FString ErrorStr;

			GEngine->Browse(*WorldContext, URL, ErrorStr);
		}
	}
	else ClientTravel(LobbyMapName, ETravelType::TRAVEL_Absolute);

	// Session will be closed on return to lobby
}

void AGamePlayerController::ChangeInputMode(bool UIOnly)
{
	if (UIOnly)
	{
		FInputModeUIOnly InputModeData = FInputModeUIOnly();
		this->SetInputMode(InputModeData);
	}
	else
	{
		FInputModeGameOnly InputModeData = FInputModeGameOnly();
		this->SetInputMode(InputModeData);
	}

	this->bShowMouseCursor = UIOnly;
}

AGameplayHUD* AGamePlayerController::GetGameplayHUD()
{
	if(!IsValid(GameplayHUD)) GameplayHUD = GetHUD<AGameplayHUD>();
	return GameplayHUD;
}

void AGamePlayerController::Server_PlayerWantsToSpectate_Implementation()
{
	StartSpectatingOnly();
}

// Input Bindings

const FName AGamePlayerController::HorizontalAxisBindingName("AxisHorizontal");
const FName AGamePlayerController::VerticalAxisBindingName("AxisVertical");
const FName AGamePlayerController::MoveForwardAxisBindingName("MoveForward");
const FName AGamePlayerController::MoveRightAxisBindingName("MoveRight");
const FName AGamePlayerController::PrimaryActionAxisBindingName("AxisPrimaryAction");
const FName AGamePlayerController::SecondaryActionAxisBindingName("AxisSecondaryAction");

const FName AGamePlayerController::MenuActionBindingName("Menu");