// Aleksandr Smirnov 2021


#include "Slate/SGameplayMainHUDWidget.h"

#include "SlateOptMacros.h"

#include "Slate/GameplayMainHUDWidgetStyle.h"
#include "General/States/GameplayGameState.h"

#define LOCTEXT_NAMESPACE "GameplayHUD"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SGameplayMainHUDWidget::Construct(const FArguments& InArgs)
{
	auto Style = InArgs._MainStyle->WidgetStyle;

	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SVerticalBox)

		// Left and right blocks of text
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)

			// Left side text block
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Left)
			.Padding(50.f, 50.f, 0, 0)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SAssignNew(WarmupHint, STextBlock)
					.TextStyle(&Style.RegularTextStyle)
					.Justification(ETextJustify::Left)
					.Visibility(EVisibility::Hidden)
					.Text(LOCTEXT("gameplayhud.warmup", "Round will start soon..."))
				]

				+ SOverlay::Slot()
				[
					SAssignNew(DefendFlagHint, STextBlock)
					.TextStyle(&Style.RegularTextStyle)
					.Justification(ETextJustify::Left)
					.Visibility(EVisibility::Hidden)
					.Text(LOCTEXT("gameplayhud.flag_defend", "Defend the Flag"))
				]

				+ SOverlay::Slot()
				[
					SAssignNew(CaptureFlagHint, STextBlock)
					.TextStyle(&Style.RegularTextStyle)
					.Justification(ETextJustify::Left)
					.Visibility(EVisibility::Hidden)
					.Text(LOCTEXT("gameplayhud.flag_capture", "Capture the Flag"))
				]
			]
		
			// Right side text block of multiple texts
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Right)
			.Padding(0, 50.f, 50.f, 0)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)

					// Left side text block
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.TextStyle(&Style.RegularTextStyle)
						.Justification(ETextJustify::Right)
						.Text(LOCTEXT("gameplayhud.roundinfo", "Round: "))
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SAssignNew(RoundInfo, STextBlock)
						.TextStyle(&Style.RegularTextStyle)
						.Justification(ETextJustify::Right)
					]
				]

				+ SVerticalBox::Slot()
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Right)
				[
					SAssignNew(RoundTimer, STextBlock)
					.TextStyle(&Style.RegularTextStyle)
					.Justification(ETextJustify::Right)
				]
			]
		]

		// Bottom block of texts
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Fill)
		.Padding(0.f, 150.f, 0.f, 150.f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(RoundWonHint_Red, STextBlock)
				.TextStyle(&Style.RedTeamNotificationTextStyle)
				.Justification(ETextJustify::Center)
				.Visibility(EVisibility::Hidden)
				.Text(LOCTEXT("gameplayhud.roundwon_red", "Red team won the round"))
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(RoundWonHint_Blue, STextBlock)
				.TextStyle(&Style.BlueTeamNotificationTextStyle)
				.Justification(ETextJustify::Center)
				.Visibility(EVisibility::Hidden)
				.Text(LOCTEXT("gameplayhud.roundwon_blue", "Blue team won the round"))
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(GameWonHint_Red, STextBlock)
				.TextStyle(&Style.RedTeamNotificationTextStyle)
				.Justification(ETextJustify::Center)
				.Visibility(EVisibility::Hidden)
				.Text(LOCTEXT("gameplayhud.gamewon_red", "Red Team won. Starting new game..."))
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(GameWonHint_Blue, STextBlock)
				.TextStyle(&Style.BlueTeamNotificationTextStyle)
				.Justification(ETextJustify::Center)
				.Visibility(EVisibility::Hidden)
				.Text(LOCTEXT("gameplayhud.gamewon_blue", "Blue Team won. Starting new game..."))
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE

void SGameplayMainHUDWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	TimeElapsedSinceCountdownUpdate += InDeltaTime;
	if (TimeElapsedSinceCountdownUpdate >= 1)
	{
		TimeElapsedSinceCountdownUpdate = 0;
		RoundTimeRemaining = FMath::Max(0, RoundTimeRemaining - 1);
		UpdateCountdown();
	}
}

void SGameplayMainHUDWidget::UpdateCountdown()
{
	RoundTimer.Get()->SetText(FString::FromInt(RoundTimeRemaining));
}

void SGameplayMainHUDWidget::UpdateWidgetData(const FMatchData& MatchData, const struct FMatchParameters& MatchParams, float TimePassed)
{
	// Setting up countdown
	uint16 StartCounterValue = (uint8)TimePassed;
	TimeElapsedSinceCountdownUpdate = TimePassed - (float)StartCounterValue;

	if (MatchData.MatchState == EMatchState::Warmup)
	{
		RoundTimeRemaining = MatchParams.WarmupPeriodSec;
	}
	else if (MatchData.MatchState == EMatchState::Gameplay)
	{
		RoundTimeRemaining = MatchParams.MatchPeriodSec;
	}
	else if (MatchData.MatchState == EMatchState::RoundEnd)
	{
		RoundTimeRemaining = MatchParams.EndRoundPeriodSec;
	}

	UpdateCountdown();
	//

	RoundInfo.Get()->SetText(MatchData.GetRoundProgressString());

	// Hiding or showing text blocks related to current match state
	WarmupHint.Get()->SetVisibility(MatchData.MatchState == EMatchState::Warmup ? EVisibility::Visible : EVisibility::Collapsed);
	
	
	//DefendFlagHint.Get()
	//CaptureFlagHint.Get()
	
	RoundWonHint_Red.Get()->SetVisibility(MatchData.SpecialMessage == EInGameSpecialMessage::RedTeamWonLastRound ? EVisibility::Visible : EVisibility::Collapsed);
	RoundWonHint_Blue.Get()->SetVisibility(MatchData.SpecialMessage == EInGameSpecialMessage::BlueTeamWonLastRound ? EVisibility::Visible : EVisibility::Collapsed);
	GameWonHint_Red.Get()->SetVisibility(MatchData.SpecialMessage == EInGameSpecialMessage::RedTeamWonLastGame ? EVisibility::Visible : EVisibility::Collapsed);
	GameWonHint_Blue.Get()->SetVisibility(MatchData.SpecialMessage == EInGameSpecialMessage::BlueTeamWonLastGame ? EVisibility::Visible : EVisibility::Collapsed);
}