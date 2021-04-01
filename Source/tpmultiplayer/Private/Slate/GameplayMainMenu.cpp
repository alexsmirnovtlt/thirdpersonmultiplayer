// Aleksandr Smirnov 2021


#include "Slate/GameplayMainMenu.h"

#include "SlateOptMacros.h"
#include "Widgets/Images/SImage.h"

#include "General/Controllers/GamePlayerController.h"
#include "Slate/GameplayMainMenuWidgetStyle.h"

#define LOCTEXT_NAMESPACE "MainMenu"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SGameplayMainMenuWidget::Construct(const FArguments& InArgs)
{
	PlayerController = InArgs._PlayerController;
	auto& Style = InArgs._MainMenuStyle->WidgetStyle;

	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SOverlay)

		+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SImage)
			.ColorAndOpacity(Style.BackgroundColor)
			.Image(&Style.BackgroundBrush)
		]

		+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(500.f)
			.HeightOverride(300.f)
			[
				SNew(SVerticalBox)
				
				+ SVerticalBox::Slot()
				.Padding(0, 0, 0, 30.f)
				[
					SAssignNew(PlayButton, SButton)
					.OnClicked_Raw(this, &SGameplayMainMenuWidget::OnPlayButtonPress)
					.VAlign(VAlign_Center)
					.Content()
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.TextStyle(&Style.TextStyle)
						.Text(LOCTEXT("mainmenu.play", "Play"))
					]
				]

				+ SVerticalBox::Slot()
				.Padding(0, 0, 0, 30.f)
				[
					SAssignNew(SpectateButton, SButton)
					.OnClicked_Raw(this, &SGameplayMainMenuWidget::OSpectateButtonPress)
					.VAlign(VAlign_Center)
					.Content()
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.TextStyle(&Style.TextStyle)
						.Text(LOCTEXT("mainmenu.spectate", "Spectate"))
					]
				]

				+ SVerticalBox::Slot()
				[
					SAssignNew(ToLobbyButton, SButton)
					.OnClicked_Raw(this, &SGameplayMainMenuWidget::OnBackToLobbyButtonPress)
					.VAlign(VAlign_Center)
					.Content()
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.TextStyle(&Style.TextStyle)
						.Text(LOCTEXT("mainmenu.tolobby", "Back To Lobby"))
					]
				]
			]
		]
	];

	EnableButtons();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE

FReply SGameplayMainMenuWidget::OnPlayButtonPress()
{
	if (!PlayerController.IsValid()) return FReply::Handled();
	
	DisableButtons();
	PlayerController.Get()->JoinGameAsPlayer();
	return FReply::Handled();
}

FReply SGameplayMainMenuWidget::OSpectateButtonPress()
{
	if (!PlayerController.IsValid()) return FReply::Handled();
	
	DisableButtons();
	PlayerController.Get()->JoinGameAsSpectator();
	return FReply::Handled();
}

FReply SGameplayMainMenuWidget::OnBackToLobbyButtonPress()
{
	if (!PlayerController.IsValid()) return FReply::Handled();

	DisableButtons();
	PlayerController.Get()->ReturnToLobby();
	return FReply::Handled();
}

void SGameplayMainMenuWidget::EnableButtons()
{
	if (!PlayerController.IsValid()) return;

	FName CurrentState = PlayerController.Get()->StateName;
	PlayButton.Get()->SetEnabled(!CurrentState.IsEqual(NAME_Playing));
	SpectateButton.Get()->SetEnabled(!CurrentState.IsEqual(NAME_Spectating));
	ToLobbyButton.Get()->SetEnabled(true);
}

void SGameplayMainMenuWidget::DisableButtons()
{
	PlayButton.Get()->SetEnabled(false);
	SpectateButton.Get()->SetEnabled(false);
	ToLobbyButton.Get()->SetEnabled(false);
}