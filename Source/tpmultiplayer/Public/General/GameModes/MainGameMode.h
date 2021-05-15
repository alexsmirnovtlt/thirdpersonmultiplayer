// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameplayTagContainer.h"

#include "MainGameMode.generated.h"

enum class ETeamType : uint8;
enum class EInGameFlagState : uint8;
enum class EAreaState : uint8;

/**
 * Handles login and logout of Player Controllers and executes main server game match logic
 */
UCLASS()
class TPMULTIPLAYER_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AMainGameMode();
	virtual void StartPlay() override;
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override { return Player; }; // We managing spawns differently, when player wants to posess a pawn, it is already somewhere in a game
	virtual void Logout(AController* Exiting) override;

public:
	void AddPlayerToAMatch(class AGamePlayerController* PlayerController);
	void RemovePlayerFromAMatch(class AGamePlayerController* PlayerController);

	void ApplyShootDamageToAPawn(class AThirdPersonCharacter* DamagedPawn);
	void OnAreaStateChanged(EAreaState AreaState);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Mode")
	const TArray<class AGameplayFlagArea*>& GetVIPAreas() const { return FlagPlacements; };

	uint8 GetTeamTypeForNewController(const class AGameplayAIController* PawnlessController);

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Classes")
	TSubclassOf<class AThirdPersonCharacter> GameplayPawnClass_RedTeam;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Classes")
	TSubclassOf<class AThirdPersonCharacter> GameplayPawnClass_BlueTeam;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Classes")
	TSubclassOf<class AGameplayAIController> AIControllerClass;

	AActor* SpectatorSpawn;
	TArray<class AGameplayPlayerStart*> TeamSpawnLocations;
	TArray<class AGameplayFlagArea*> FlagPlacements;

	TArray<class AThirdPersonCharacter*> TeamPawns;

	int32 HumanPlayersCount_RedTeam = 0;
	int32 HumanPlayersCount_BlueTeam = 0;

	TArray<class AGameplayAIController*> InGameControllers_AI;
	TArray<class AGamePlayerController*> InGameControllers_Human;

	// Inital setup logic
	void SetupSpawnLocations();
	void SetupPlayableCharacters();
	void GrantGameplayAbilities();
	//

	class AGameplayGameState* GameplayState;

	// BEGIN Match logic
	void InitialMatchStateSetup();
	void ResetPawnsForNewRound();
	void DetermineTeamThatWonThatRound(struct FMatchData& CurrentMatchData);
	void GiveFlagToARandomPawn(ETeamType TeamWithFlag);

	void MatchPhaseStart_Warmup();
	void MatchPhaseStart_Gameplay();
	void MatchPhaseStart_RoundEnd();

	void StopCurrentMatchTimer();
	FTimerHandle MatchTimerHandle;

	UFUNCTION()
	void OnPawnDamaged(AThirdPersonCharacter* DamagedPawn);
	void OnPawnKilled(AThirdPersonCharacter* DiedPawn);

	int32 VIPPawnIndex = -1;

	// END Match logic

	// BEGIN GAS parameters and Logic

	// Ability that allows Pawns to move, shoot, aim, etc that is activated when main game phase begins
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> WarmupPhaseEffect;
	// Opposite of ActivePawnAbility. Pawn cannot do anything. Activated before main game phase or when pawn got killed
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> MainPhaseEffect;
	// Tags to remove from ActivePawnAbility when End Round phase starts
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> EndPhaseEffect;
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> WeaponDamageEffect;
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> DeadStateEffect;
	// Permanent effect that will be applied to one random pawn at the start of the round. This pawn is able to capture game zones
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> ZoneCaptureEffect;
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayAbility> ShootAbility;
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayAbility> AimAbility;
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayAbility> ReloadAbility;
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<class UGameplayAbility> SprintAbility;

	void ApplyGameplayEffectToAllPawns(class UGameplayEffect* GEffectToAddPtr);

	// END GAS parameters and Logic
	
	static const FString NewPlayerOptionsNameKey; // When new PlayerController gets created, set its name from option parameter with that key name on AMainGameMode::Login() 

public:
	// DEBUG
	void Debug_KillRandomPawn();
	//

private:
	int32 GetNextSpawnLocationIndex(int32 StartingIndex, ETeamType TeamType);
	int32 GetNextPlayerControllerIndex(int32 StartingIndex, ETeamType TeamType);
	int32 GetNextUnpossessedPawnIndex(int32 StartingIndex, ETeamType TeamType);
};
