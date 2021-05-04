// Aleksandr Smirnov 2021


#include "General/Animation/StepAnimNotify_FMOD.h"

#include "FMODBlueprintStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UStepAnimNotify_FMOD::UStepAnimNotify_FMOD()
    : Super()
{

#if WITH_EDITORONLY_DATA
    NotifyColor = FColor(196, 142, 255, 255);
#endif // WITH_EDITORONLY_DATA

    MaxSoundDistance = 500.f;
    MinVolume = 0.1f;
    MaxVolume = 0.8f;
}

void UStepAnimNotify_FMOD::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* AnimSeq)
{
    int32 ChosenIndex = FMath::RandRange(0, Events.Num() - 1);
    if (ChosenIndex == Events.Num()) return; // Array is empty
   
    UFMODEvent* ChosenEvent = Events[ChosenIndex];
    if (!ChosenEvent) { UE_LOG(LogTemp, Warning, TEXT("UStepAnimNotify_FMOD::Notify TSoftObjPtr returned null!")); return; }
   
    auto CreatedAudioComponent = UFMODBlueprintStatics::PlayEventAttached(ChosenEvent, MeshComp, *BoneName, FVector(0, 0, 0), EAttachLocation::KeepRelativeOffset, true, false, true);
    CreatedAudioComponent->SetVolume(FMath::RandRange(MinVolume, MaxVolume));
    CreatedAudioComponent->AttenuationDetails.MaximumDistance = MaxSoundDistance;
    CreatedAudioComponent->Play();
}