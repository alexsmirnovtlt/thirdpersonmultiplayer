// Aleksandr Smirnov 2021


#include "General/Controllers/GamePlayerController.h"

#include "GameFramework/SpectatorPawn.h"
#include "GameFramework/Pawn.h"

#include "General/MultiplayerGameInstance.h"
#include "General/HUD/GameplayHUD.h"

void AGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		GameplayHUD = GetHUD<AGameplayHUD>();

		if (IsValid(InputComponent))
			InputComponent->BindAction(MenuActionBindingName, EInputEvent::IE_Pressed, this, &AGamePlayerController::MenuActionInput);

		ChangeInputMode(true); // Changing input to menu because HUD will spawn it
	}
}

void AGamePlayerController::EndPlay(EEndPlayReason::Type Type)
{
	Super::EndPlay(Type);

	
}

void AGamePlayerController::JoinGameAsPlayer()
{
	GameplayHUD->MainMenu_Hide();
	ChangeInputMode(false);

	Server_PlayerWantsToPlay();
}

void AGamePlayerController::JoinGameAsSpectator()
{
	GameplayHUD->MainMenu_Hide();
	ChangeInputMode(false);
	
	StartSpectatingOnly();
	if(!HasAuthority()) Server_PlayerWantsToSpectate();
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

void AGamePlayerController::ChangeInputMode(bool bMenuMode)
{
	auto CurrentPawn = GetSpectatorPawn();

	if (bMenuMode)
	{
		if (IsValid(CurrentPawn)) CurrentPawn->DisableInput(this);
		FInputModeGameAndUI InputModeData = FInputModeGameAndUI();
		this->SetInputMode(InputModeData);
	}
	else
	{
		if (IsValid(CurrentPawn)) CurrentPawn->EnableInput(this);
		FInputModeGameOnly InputModeData = FInputModeGameOnly();
		this->SetInputMode(InputModeData);
	}

	this->bShowMouseCursor = bMenuMode;
}

void AGamePlayerController::MenuActionInput()
{
	if (IsValid(GameplayHUD)) GameplayHUD->MainMenu_Toggle();
}

// BEGIN Server logic

void AGamePlayerController::Server_PlayerWantsToPlay_Implementation()
{
	// Player Trying to join a Match as a player

	// TODO Add more logic
}

void AGamePlayerController::Server_PlayerWantsToSpectate_Implementation()
{
	// Player Joined a Match as a spectator

	StartSpectatingOnly(); // Spectator pawn can only be spawned locally, so this was executed on a client and then on a server so PlayerState parameters would be updated on a server
	
	// TODO Add more logic
}

// END Server logic

// BEGIN Input Bindings

// Axes
const FName AGamePlayerController::HorizontalAxisBindingName("AxisHorizontal");
const FName AGamePlayerController::VerticalAxisBindingName("AxisVertical");
const FName AGamePlayerController::MoveForwardAxisBindingName("MoveForward");
const FName AGamePlayerController::MoveRightAxisBindingName("MoveRight");
const FName AGamePlayerController::PrimaryActionAxisBindingName("AxisPrimaryAction");
const FName AGamePlayerController::SecondaryActionAxisBindingName("AxisSecondaryAction");

// Actions
const FName AGamePlayerController::MenuActionBindingName("Menu");

// END Input Bindings