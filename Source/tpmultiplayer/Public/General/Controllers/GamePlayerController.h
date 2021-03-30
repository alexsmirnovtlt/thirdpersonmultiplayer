// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GamePlayerController.generated.h"

/**
 * Main Player Controller
 */
UCLASS()
class TPMULTIPLAYER_API AGamePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void BeginPlay() override;
	void EndPlay(EEndPlayReason::Type Type) override;

public:
	void JoinGameAsPlayer();
	void JoinGameAsSpectator();
	void ReturnToLobby();

	class AGameplayHUD* GetGameplayHUD() { return GameplayHUD; };

	void ChangeInputMode(bool bMenuMode);

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

	// BEGIN Input 

public:

	static const FName HorizontalAxisBindingName;
	static const FName VerticalAxisBindingName;
	static const FName MoveForwardAxisBindingName;
	static const FName MoveRightAxisBindingName;
	static const FName PrimaryActionAxisBindingName;
	static const FName SecondaryActionAxisBindingName;
	static const FName MenuActionBindingName;

	void MenuActionInput();

	// END Input
};
