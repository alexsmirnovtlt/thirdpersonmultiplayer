// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "General/GameplayStructs.h"
#include "GameFramework/PlayerController.h"

#include "GamePlayerController.generated.h"

enum class ETeamType : uint8;

/**
 * Main Player Controller
 */
UCLASS()
class TPMULTIPLAYER_API AGamePlayerController : public APlayerController
{
	GENERATED_BODY()

	friend class AMainGameMode;

public:
	AGamePlayerController();

	void BeginPlay() override;
	void OnRep_Pawn() override;
	void PawnLeavingGame() override {}; // Pawn will not be destroyed
	void AcknowledgePossession(class APawn* P) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void JoinGameAsPlayer();
	void JoinGameAsSpectator();
	void ReturnToLobby();

	ETeamType GetTeamType() const { return TeamType; };
	class AGameplayHUD* GetGameplayHUD() const { return GameplayHUD; };
	class AGameplayGameState* GetGameplayState() const { return GameplayState; };

	void ChangeInputMode(bool bMenuMode);

	UFUNCTION(Client, Unreliable)
	void Client_ReplicateShot(const struct FShootData& ShootData);
	UFUNCTION(Client, Unreliable)
	void Client_ReplicateReload(AActor* ReloadingActor);

protected:

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_PlayerWantsToSpectate();
	void Server_PlayerWantsToSpectate_Implementation();
	bool Server_PlayerWantsToSpectate_Validate() { return true; };

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_PlayerWantsToPlay();
	void Server_PlayerWantsToPlay_Implementation();
	bool Server_PlayerWantsToPlay_Validate() { return true; };

	class AGameplayHUD* GameplayHUD;
	class AGameplayGameState* GameplayState;
	
	UPROPERTY(Replicated)
	ETeamType TeamType;

	// BEGIN Input 

public:

	static const FName HorizontalAxisBindingName;
	static const FName VerticalAxisBindingName;
	static const FName MoveForwardAxisBindingName;
	static const FName MoveRightAxisBindingName;

	static const FName MenuActionBindingName;
	static const FName GamePlayHUDBindingName;
	static const FName AdditionalActionBindingName;
	static const FName SwitchShoulderBindingName;
	static const FName ShootBindingName;
	static const FName AimBindingName;
	static const FName ReloadBindingName;
	static const FName SprintBindingName;
	static const FName AbilityConfirmBindingName;
	static const FName AbilityCancelBindingName;
	static const FName DebugKillBindingName;

	void MenuActionInput();
	void HUDToggleActionInput();
	void Debug_KillRandomPawn();

	// END Input
};
