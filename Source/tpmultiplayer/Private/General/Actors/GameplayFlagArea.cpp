// Aleksandr Smirnov 2021


#include "General/Actors/GameplayFlagArea.h"

#include "Net/UnrealNetwork.h"

#include "General/Pawns/ThirdPersonCharacter.h"
#include "General/States/GameplayGameState.h"
#include "General/GameModes/MainGameMode.h"

AGameplayFlagArea::AGameplayFlagArea()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.5;

	NetUpdateFrequency = 0; // TODO probably needs to change

	bReplicates = true;
}

void AGameplayFlagArea::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(false); // Will only tick if area is being captured

	if(!HasAuthority()) OnGetInactive(); // Call to BP
}

void AGameplayFlagArea::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CaptureTimeProgressSec > AreaCaptureTimeSec)
		AreaIsCaptured(); // Team with a flag captured this area
	else
	{
		CaptureTimeProgressSec += DeltaTime;
		OnRep_CaptureProgressChanged();
	}
}

void AGameplayFlagArea::InitialSetup(AMainGameMode* GameModePtr, AGameplayGameState* GameStatePtr)
{
	MainGameMode = GameModePtr;
	GameState = GameStatePtr;

	AreaCaptureTimeSec = GameState->GetMatchParameters().FlagDefenseTime;
	
	ResetFlagState();
}

void AGameplayFlagArea::ResetFlagState()
{
	SetActorTickEnabled(false);

	CaptureTimeProgressSec = 0.f;
	FlagOwningTeam = GameState->GetCurrentMatchData().RedTeamHasFlag ? ETeamType::RedTeam : ETeamType::BlueTeam;

	OnFlagStateReset(); // call to BP
}

void AGameplayFlagArea::AreaIsCaptured()
{
	SetActorTickEnabled(false);

	if (GameState->GetCurrentMatchData().MatchState == EMatchState::Gameplay)
		MainGameMode->OnAreaStateChanged(EAreaState::Captured);
}

void AGameplayFlagArea::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	auto Player = Cast<AThirdPersonCharacter>(OtherActor);
	if (!Player) return;

	if (Player->IsVIP())  // TODO Maybe check sould be different (GAS?)
	{
		MainGameMode->OnAreaStateChanged(EAreaState::BeingCaptured);
		SetActorTickEnabled(true);
	}
}

void AGameplayFlagArea::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	if (GameState->GetCurrentMatchData().MatchState != EMatchState::Gameplay) return;

	auto Player = Cast<AThirdPersonCharacter>(OtherActor);
	if (!Player) return;

	if (Player->IsVIP()) // TODO Maybe check sould be different (GAS?)
	{
		MainGameMode->OnAreaStateChanged(EAreaState::Default);
		CaptureTimeProgressSec = 0;
		SetActorTickEnabled(false); 
	}
}

void AGameplayFlagArea::OnRep_CaptureProgressChanged()
{
	UpdateCaptureProgress((CaptureTimeProgressSec + 0.5f) / AreaCaptureTimeSec * 100); // Call to BP. +0.5 means that progress bar will actually be able to reach 100%
}

bool AGameplayFlagArea::IsRedTeamOwnsFlag()
{ 
	return FlagOwningTeam == ETeamType::RedTeam;
}

void AGameplayFlagArea::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGameplayFlagArea, CaptureTimeProgressSec);
}