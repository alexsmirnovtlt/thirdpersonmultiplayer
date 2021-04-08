// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayFlagArea.generated.h"

UCLASS()
class TPMULTIPLAYER_API AGameplayFlagArea : public AActor
{
	GENERATED_BODY()
	
public:	
	AGameplayFlagArea();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	void NotifyActorBeginOverlap(AActor* OtherActor) override;
	void NotifyActorEndOverlap(AActor* OtherActor) override;
	//void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

public:
	void InitialSetup(class AMainGameMode* GameModePtr, class AGameplayGameState* GameStatePtr);
	void ResetFlagState();

	UFUNCTION(BlueprintImplementableEvent, Category = "Flag Actor")
	void OnFlagStateReset();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Flag Variables")
	class AMainGameMode* MainGameMode;
	UPROPERTY(BlueprintReadOnly, Category = "Flag Variables")
	class AGameplayGameState* GameState;
};
