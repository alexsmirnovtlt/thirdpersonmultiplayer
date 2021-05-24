// Aleksandr Smirnov 2021


#include "Slate/SGameplayMainMenu.h"

#include "SlateOptMacros.h"
#include "Widgets/Images/SImage.h"

#include "General/Controllers/GamePlayerController.h"
#include "Slate/Styles/GameplayMainMenuWidgetStyle.h"
#include "General/States/GameplayGameState.h"
#include "General/HUD/GameplayHUD.h"

#define LOCTEXT_NAMESPACE "MainMenu"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SGameplayMainMenuWidget::Construct(const FArguments& InArgs)
{
	PlayerController = InArgs._PlayerController;
	PlayerHUD = InArgs._PlayerHUD;

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
					.OnClicked(this, &SGameplayMainMenuWidget::OnPlayButtonPress)
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
					.OnClicked(this, &SGameplayMainMenuWidget::OSpectateButtonPress)
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
					.OnClicked(this, &SGameplayMainMenuWidget::OnBackToLobbyButtonPress)
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

		+ SOverlay::Slot()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Right)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.Padding(0, 0, 30.f, 0)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Left)
				.TextStyle(&Style.HintTextStyle)
				.Text(LOCTEXT("mainmenu.tutorial.1", "Keybinds:"))
			]

			+ SVerticalBox::Slot()
				.Padding(0, 0, 30.f, 0)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Left)
				.TextStyle(&Style.HintTextStyle)
				.Text(LOCTEXT("mainmenu.tutorial.2", "- WASD to move, RMB to AIM, LMB to Shoot"))
			]

			+ SVerticalBox::Slot()
			.Padding(0, 0, 30.f, 0)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Left)
				.TextStyle(&Style.HintTextStyle)
				.Text(LOCTEXT("mainmenu.tutorial.3", "- Middle mouse to switch shoulder"))
			]

			+ SVerticalBox::Slot()
			.Padding(0, 0, 30.f, 0)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Left)
				.TextStyle(&Style.HintTextStyle)
				.Text(LOCTEXT("mainmenu.tutorial.4", "- R reload, Shift to sprint"))
			]

			+ SVerticalBox::Slot()
				.Padding(0, 0, 30.f, 10.f)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Left)
				.TextStyle(&Style.HintTextStyle)
				.Text(LOCTEXT("mainmenu.tutorial.5", "- U Hide/show UI"))
			]
		]

		+ SOverlay::Slot()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Left)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.Padding(30.f, 0, 0, 0)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Left)
				.TextStyle(&Style.HintTextStyle)
				.Text(LOCTEXT("mainmenu.rules.1", "Game Rules:"))
			]

			+ SVerticalBox::Slot()
			.Padding(30.f, 0, 0, 0)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Left)
				.TextStyle(&Style.HintTextStyle)
				.Text(LOCTEXT("mainmenu.rules.2", "- Map has 2 VIP areas"))
			]
			
			+ SVerticalBox::Slot()
			.Padding(30.f, 0, 0, 0)
			[
					SNew(STextBlock)
					.Justification(ETextJustify::Left)
					.TextStyle(&Style.HintTextStyle)
					.Text(LOCTEXT("mainmenu.rules.3", "- Player with a star above him (VIP) need to stay in this area for some time to win"))
			]
			
			+ SVerticalBox::Slot()
			.Padding(30.f, 0, 0, 10.f)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Left)
				.TextStyle(&Style.HintTextStyle)
				.Text(LOCTEXT("mainmenu.rules.4", "- You can also win if there is no enemies alive"))
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
	
	if (PlayerHUD.IsValid()) PlayerHUD.Get()->PlayButtonClickSound();

	DisableButtons();
	PlayerController.Get()->JoinGameAsPlayer();
	return FReply::Handled();
}

FReply SGameplayMainMenuWidget::OSpectateButtonPress()
{
	if (!PlayerController.IsValid()) return FReply::Handled();

	if (PlayerHUD.IsValid()) PlayerHUD.Get()->PlayButtonClickSound();

	DisableButtons();
	PlayerController.Get()->JoinGameAsSpectator();
	return FReply::Handled();
}

FReply SGameplayMainMenuWidget::OnBackToLobbyButtonPress()
{
	if (!PlayerController.IsValid()) return FReply::Handled();

	if (PlayerHUD.IsValid()) PlayerHUD.Get()->PlayButtonClickSound();

	DisableButtons();
	PlayerController.Get()->ReturnToLobby();
	return FReply::Handled();
}

void SGameplayMainMenuWidget::EnableButtons()
{
	if (!PlayerController.IsValid()) return;

	bool IsSpectator = true;

	IsSpectator = PlayerController.Get()->GetTeamType() == ETeamType::Spectator;

	PlayButton.Get()->SetEnabled(IsSpectator);
	SpectateButton.Get()->SetEnabled(!IsSpectator);
	ToLobbyButton.Get()->SetEnabled(true);
}

void SGameplayMainMenuWidget::DisableButtons()
{
	PlayButton.Get()->SetEnabled(false);
	SpectateButton.Get()->SetEnabled(false);
	ToLobbyButton.Get()->SetEnabled(false);
}