// Aleksandr Smirnov 2021


#include "General/Controllers/GamePlayerController.h"

#include "GameFramework/SpectatorPawn.h"
#include "GameFramework/Pawn.h"

#include "General/States/GameplayGameState.h"
#include "General/MultiplayerGameInstance.h"
#include "General/GameModes/MainGameMode.h"
#include "General/HUD/GameplayHUD.h"

void AGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		GameplayHUD = GetHUD<AGameplayHUD>();
		GameplayState = GetWorld()->GetGameState<AGameplayGameState>();
		if (!GameplayHUD || !GameplayState) { ensure(GameplayHUD && GameplayState); return; }

		if (IsValid(InputComponent))
		{
			InputComponent->BindAction(MenuActionBindingName, EInputEvent::IE_Pressed, this, &AGamePlayerController::MenuActionInput);
			InputComponent->BindAction(GamePlayHUDBindingName, EInputEvent::IE_Pressed, this, &AGamePlayerController::HUDToggleActionInput);
		}

		ChangeInputMode(true); // Changing input to menu because HUD will spawn it
	}
}

void AGamePlayerController::EndPlay(EEndPlayReason::Type Type)
{
	Super::EndPlay(Type);

	
}

void AGamePlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	GameplayHUD->MainMenu_Hide();
	GameplayHUD->GameplayMenu_Show();

	ChangeInputMode(false);

	//ChangeState(NAME_Playing); // TODO may be redundant. Needs check when it happens already
}

void AGamePlayerController::JoinGameAsPlayer()
{
	Server_PlayerWantsToPlay();
}

void AGamePlayerController::JoinGameAsSpectator()
{
	if (IsInState(NAME_Inactive))
	{
		// Its our first join as a spectator that has no pawn. Without this check spectator will probably spawn at FVector::ZeroVector with zero rotation. We have a special Player Start for that
		auto GameState = GetWorld()->GetGameState<AGameplayGameState>();
		ControlRotation = GameState->GetSpectatorInitialSpawnRotation(); // Spectator pawn`s rotation is set from Control Rotation
		ChangeState(NAME_Spectating); // Creating and posessing local Spectator Pawn
		GetSpectatorPawn()->SetActorLocation(GameState->GetSpectatorInitialSpawnLocation()); // Manually setting spectator`s location
	}
	else if (IsInState(NAME_Playing))
	{
		
	}
	else return; // Somehow we are already spectating, do nothing

	Server_PlayerWantsToSpectate(); // Server will update state to spectating

	ChangeInputMode(false);
	GameplayHUD->MainMenu_Hide();
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

// BEGIN Server logic

void AGamePlayerController::Server_PlayerWantsToPlay_Implementation()
{
	// Player Trying to join a Match as a player
	if (auto AuthGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>())
	{
		AuthGameMode->AddPlayerToAMatch(this);
		ChangeState(NAME_Playing);
	}
}

void AGamePlayerController::Server_PlayerWantsToSpectate_Implementation()
{
	if (IsInState(NAME_Playing))
	{
		if (auto AuthGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>())
			AuthGameMode->RemovePlayerFromAMatch(this);
	}

	ChangeState(NAME_Spectating);
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
const FName AGamePlayerController::GamePlayHUDBindingName("HUDToggle");

void AGamePlayerController::MenuActionInput()
{
	if (IsValid(GameplayHUD)) GameplayHUD->MainMenu_Toggle();
}

void AGamePlayerController::HUDToggleActionInput()
{
	if (IsValid(GameplayHUD)) GameplayHUD->GameplayMenu_Toggle();
}

// END Input Bindings