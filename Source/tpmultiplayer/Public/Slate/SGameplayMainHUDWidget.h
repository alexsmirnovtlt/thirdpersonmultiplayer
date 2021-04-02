// Aleksandr Smirnov 2021

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class TPMULTIPLAYER_API SGameplayMainHUDWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGameplayMainHUDWidget)
	{}
	SLATE_ARGUMENT(const class UGameplayMainHUDWidgetStyle*, MainStyle)
	//SLATE_ARGUMENT(TWeakObjectPtr<class AGameplayGameState>, GameState)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void UpdateWidgetData(const struct FMatchData& MatchData, const struct FMatchParameters& MatchParams, float TimePassed);
	void UpdateCountdown();

protected:

	TWeakObjectPtr<class AGameplayGameState> GameState;

	// Left side text block references
	TSharedPtr<class STextBlock> WarmupHint;
	TSharedPtr<class STextBlock> DefendFlagHint;
	TSharedPtr<class STextBlock> CaptureFlagHint;
	// Right side text block references
	TSharedPtr<class STextBlock> RoundInfo;
	TSharedPtr<class STextBlock> RoundTimer;
	// Bottom text block references
	TSharedPtr<class STextBlock> RoundWonHint_Red;
	TSharedPtr<class STextBlock> RoundWonHint_Blue;
	TSharedPtr<class STextBlock> GameWonHint_Red;
	TSharedPtr<class STextBlock> GameWonHint_Blue;

	uint16 RoundTimeRemaining;
	float TimeElapsedSinceCountdownUpdate = 0;
};
