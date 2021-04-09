// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayFlagArea.generated.h"

enum class ETeamType : uint8;

UENUM()
enum class EAreaState : uint8 {
	Default = 0,
	BeingCaptured = 1,
	Captured = 2
};

UCLASS()
class TPMULTIPLAYER_API AGameplayFlagArea : public AActor
{
	GENERATED_BODY()
	
public:	
	AGameplayFlagArea();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	void NotifyActorBeginOverlap(AActor* OtherActor) override;
	void NotifyActorEndOverlap(AActor* OtherActor) override;

public:
	void InitialSetup(class AMainGameMode* GameModePtr, class AGameplayGameState* GameStatePtr);
	void ResetFlagState();

	UFUNCTION(BlueprintImplementableEvent, Category = "Flag Actor")
	void OnFlagStateReset();
	UFUNCTION(BlueprintImplementableEvent, Category = "Flag Actor")
	void UpdateCaptureProgress(float Percentage);
	UFUNCTION(BlueprintImplementableEvent, Category = "Flag Actor")
	void OnGetInactive();
	UFUNCTION(BlueprintCallable, Category = "Flag Actor")
	bool IsRedTeamOwnsFlag();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Flag Variables")
	class AMainGameMode* MainGameMode;
	UPROPERTY(BlueprintReadOnly, Category = "Flag Variables")
	class AGameplayGameState* GameState;

	float AreaCaptureTimeSec;

	void AreaIsCaptured();

	//UPROPERTY(BlueprintReadOnly, Category = "Flag Variables")
	ETeamType FlagOwningTeam;

	UPROPERTY(ReplicatedUsing=OnRep_CaptureProgressChanged)
	float CaptureTimeProgressSec;
	UFUNCTION()
	void OnRep_CaptureProgressChanged();
};
