// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GameplayHUD.generated.h"

/**
 * 
 */
UCLASS(abstract)
class TPMULTIPLAYER_API AGameplayHUD : public AHUD
{
	GENERATED_BODY()

public:
	void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Slate")
	TSubclassOf <class UGameplayMainMenuWidgetStyle> MainMenuStyleClass;

	TSharedPtr<class SGameplayMainMenuWidget> MainMenuWidget;
	TSharedPtr<class SWeakWidget> MainMenuWidgetContainer;
};
