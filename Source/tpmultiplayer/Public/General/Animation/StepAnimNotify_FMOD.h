// Aleksandr Smirnov 2021

#pragma once

#include "FMODEvent.h"
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "StepAnimNotify_FMOD.generated.h"

/**
 * Slightly modified version of UFMODAnimNotifyPlay that can play random step sound at bone location
 */
UCLASS(abstract, const, hidecategories = Object, collapsecategories, meta = (DisplayName = "Play FMOD Event"))
class TPMULTIPLAYER_API UStepAnimNotify_FMOD : public UAnimNotify
{
	GENERATED_BODY()

public:
    UStepAnimNotify_FMOD();

    // Being UAnimNotify interface
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* AnimSeq) override;
    // End UAnimNotify interface

    // Socket or bone name to attach sound to
    UPROPERTY(EditDefaultsOnly, Category = "Setup")
    FString BoneName;

    // Sounds to Play
    UPROPERTY(EditDefaultsOnly, Category = "Setup")
    TArray<TAssetPtr<class UFMODEvent>> Events;

    UPROPERTY(EditDefaultsOnly, Category = "Setup")
    float MaxSoundDistance;

    UPROPERTY(EditDefaultsOnly, Category = "Setup")
    float MinVolume;
    UPROPERTY(EditDefaultsOnly, Category = "Setup")
    float MaxVolume;
};
