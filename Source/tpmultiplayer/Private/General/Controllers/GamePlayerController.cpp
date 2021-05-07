// Aleksandr Smirnov 2021


#include "General/Controllers/GamePlayerController.h"

#include "GameFramework/SpectatorPawn.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

#include "General/Pawns/ThirdPersonCharacter.h"
#include "General/States/GameplayGameState.h"
#include "General/MultiplayerGameInstance.h"
#include "General/GameModes/MainGameMode.h"
#include "General/HUD/GameplayHUD.h"

AGamePlayerController::AGamePlayerController()
{
	TeamType = ETeamType::Spectator;
	NetUpdateFrequency = 0;
}

void AGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
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

void AGamePlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if(IsValid(GameplayHUD)) GameplayHUD->MainMenu_Hide();
	if (!GetPawn() && !IsInState(NAME_Spectating)) ChangeState(NAME_Spectating); // Locally spawn spectator pawn 
}

void AGamePlayerController::AcknowledgePossession(class APawn* P)
{
	Super::AcknowledgePossession(P);

	if (AThirdPersonCharacter* TPCharacter = Cast<AThirdPersonCharacter>(P))
	{
		if (!HasAuthority())
		{
			// When player takes ownership of a pawn, it already has replicated effect tags that will never be removed, idk if this is a bug or intended
			// f.e if player possessed a pawn on a main phase, main phase effect tags will not be removed ever
			FGameplayTagContainer AppliedTags;
			TPCharacter->GetAbilitySystemComponent()->GetOwnedGameplayTags(AppliedTags);
			for (auto& item : AppliedTags)
				TPCharacter->GetAbilitySystemComponent()->SetTagMapCount(item, 0);
		}

		TPCharacter->GetAbilitySystemComponent()->InitAbilityActorInfo(this, TPCharacter);
	}
}

void AGamePlayerController::JoinGameAsPlayer()
{
	GameplayHUD->GameplayMenu_Show();
	Server_PlayerWantsToPlay();
}

void AGamePlayerController::JoinGameAsSpectator()
{
	GameplayHUD->GameplayMenu_Show();
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
	if (auto AuthGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>())
		AuthGameMode->RemovePlayerFromAMatch(this);
}

// END Server logic

// BEGIN Client logic

void AGamePlayerController::Client_ReplicateShot_Implementation(const FShootData& ShootData)
{
	if (ShootData.Shooter) CastChecked<AThirdPersonCharacter>(ShootData.Shooter)->OnRep_Shot(ShootData);
}

void AGamePlayerController::Client_ReplicateReload_Implementation(AActor* ReloadingActor)
{
	if (ReloadingActor) CastChecked<AThirdPersonCharacter>(ReloadingActor)->OnRep_Reload();
}

// END Client logic

void AGamePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGamePlayerController, TeamType);
}

// BEGIN Input Bindings

// Axes
const FName AGamePlayerController::HorizontalAxisBindingName("AxisHorizontal");
const FName AGamePlayerController::VerticalAxisBindingName("AxisVertical");
const FName AGamePlayerController::MoveForwardAxisBindingName("MoveForward");
const FName AGamePlayerController::MoveRightAxisBindingName("MoveRight");

// Actions
const FName AGamePlayerController::MenuActionBindingName("Menu");
const FName AGamePlayerController::GamePlayHUDBindingName("HUDToggle");
const FName AGamePlayerController::AdditionalActionBindingName("AdditionalAction");
const FName AGamePlayerController::SwitchShoulderBindingName("SwitchShoulder");
const FName AGamePlayerController::ShootBindingName("Shoot");
const FName AGamePlayerController::AimBindingName("Aim");
const FName AGamePlayerController::ReloadBindingName("Reload");
const FName AGamePlayerController::SprintBindingName("Sprint");
const FName AGamePlayerController::AbilityConfirmBindingName("AbilityConfirm");
const FName AGamePlayerController::AbilityCancelBindingName("AbilityCancel");

// Debug
const FName AGamePlayerController::DebugKillBindingName("DebugKill");

void AGamePlayerController::MenuActionInput()
{
	if (IsInState(NAME_Inactive)) return;
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