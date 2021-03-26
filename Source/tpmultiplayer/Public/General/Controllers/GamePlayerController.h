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
	void JoinGameAsPlayer();
	void JoinGameAsSpectator();
	void ReturnToLobby();
};
