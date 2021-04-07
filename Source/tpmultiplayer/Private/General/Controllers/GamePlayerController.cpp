// Aleksandr Smirnov 2021


#include "General/Controllers/GamePlayerController.h"

#include "GameFramework/SpectatorPawn.h"
#include "GameFramework/Pawn.h"

#include "General/States/GameplayPlayerState.h"
#include "General/States/GameplayGameState.h"
#include "General/MultiplayerGameInstance.h"
#include "General/GameModes/MainGameMode.h"
#include "General/HUD/GameplayHUD.h"

const float AGamePlayerController::NewControlRotationPitchOnPawnPossess = -30.f;

void AGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GamePlayerState = GetPlayerState<AGameplayPlayerState>(); // For some reason it returns nullprt here for clients, so thay will get it in OnRep_PlayerState()
		if (!GamePlayerState) { ensure(false); return; }
	}

	if (IsLocalController())
	{
		GameplayHUD = GetHUD<AGameplayHUD>();
		GameplayState = GetWorld()->GetGameState<AGameplayGameState>();

		if (!GameplayHUD || !GameplayState) { ensure(false); return; }

		if (IsValid(InputComponent))
		{
			InputComponent->BindAction(MenuActionBindingName, EInputEvent::IE_Pressed, this, &AGamePlayerController::MenuActionInput);
			InputComponent->BindAction(GamePlayHUDBindingName, EInputEvent::IE_Pressed, this, &AGamePlayerController::HUDToggleActionInput);

			if(HasAuthority()) InputComponent->BindAction(DebugKillBindingName, EInputEvent::IE_Pressed, this, &AGamePlayerController::Debug_KillRandomPawn);
		}
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
	ChangeInputMode(false);

	if (GetPawn())
	{
		ChangeState(NAME_Playing); // TODO probably redundant state set

		// Setting control rotation so camera will be set behind the character
		FRotator NewControlRotation = GetPawn()->GetActorRotation();
		NewControlRotation.Roll = 0.f;
		NewControlRotation.Add(NewControlRotationPitchOnPawnPossess, 0, 0);
		ControlRotation = NewControlRotation;
	}
	else ChangeState(NAME_Spectating);
}

void AGamePlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	GamePlayerState = GetPlayerState<AGameplayPlayerState>();
	if (!GamePlayerState) ensure(false);
}

void AGamePlayerController::JoinGameAsPlayer()
{
	GameplayHUD->GameplayMenu_Show();
	Server_PlayerWantsToPlay();
}

void AGamePlayerController::JoinGameAsSpectator()
{
	GameplayHUD->GameplayMenu_Show();

	if (IsInState(NAME_Spectating)) { UE_LOG(LogTemp, Warning, TEXT("AGamePlayerController::JoinGameAsSpectator() called while already a spectator!")); return; }

	if (IsInState(NAME_Inactive))
	{
		// Its our first join as a spectator that has no pawn. Without this check spectator will probably spawn at FVector::ZeroVector with zero rotation. We have a special Player Start for that
		auto GameState = GetWorld()->GetGameState<AGameplayGameState>();
		ControlRotation = GameState->GetSpectatorInitialSpawnRotation(); // Spectator pawn`s rotation is set from Control Rotation
		ChangeState(NAME_Spectating); // Creating and posessing local Spectator Pawn
		GetSpectatorPawn()->SetActorLocation(GameState->GetSpectatorInitialSpawnLocation()); // Manually setting spectator`s location

		ChangeInputMode(false);
		GameplayHUD->MainMenu_Hide();
	}
	
	Server_PlayerWantsToSpectate(); // Server will update state to spectating
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
		AuthGameMode->AddPlayerToAMatch(this);
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
const FName AGamePlayerController::AdditionalActionBindingName("AdditionalAction");
const FName AGamePlayerController::DebugKillBindingName("DebugKill");

void AGamePlayerController::MenuActionInput()
{
	if (IsValid(GameplayHUD)) GameplayHUD->MainMenu_Toggle();
}

void AGamePlayerController::HUDToggleActionInput()
{
	if (IsValid(GameplayHUD)) GameplayHUD->GameplayMenu_Toggle();
}

void AGamePlayerController::Debug_KillRandomPawn()
{
	if (auto AuthGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>())
		AuthGameMode->Debug_KillRandomPawn();
}

// END Input Bindings