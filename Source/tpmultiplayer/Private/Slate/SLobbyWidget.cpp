// Aleksandr Smirnov 2021


#include "Slate/SLobbyWidget.h"
#include "SlateOptMacros.h"

#include "Widgets/Input/SEditableText.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Images/SImage.h"

#include "General/Gamemodes/LobbyGameMode.h"

#define LOCTEXT_NAMESPACE "LobbyWidget"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLobbyWidget::Construct(const FArguments& InArgs)
{
	LobbyGameMode = InArgs._LobbyGameMode;

	auto Styles = InArgs._LobbyStyle;

	TArray<const FSlateBrush*> SlateBrushes;
	Styles->GetResources(SlateBrushes);

	const int32 LastBrushSizeIndex = SlateBrushes.Num() - 1;

	SessionName = Styles->GetSessionNameDefaultText();
	PlayerName = Styles->GetPlayerNameDefaultText();

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
			.ColorAndOpacity(Styles->BackgroundColor)
			.Image(SlateBrushes[LastBrushSizeIndex])
		]

		+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		.Padding(300.f, 100.f, 300.f, 100.f)
		[
			SNew(SHorizontalBox)

			// Left Side - Games list Scroll or Loading text
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					.VAlign(VAlign_Fill)
					.HAlign(HAlign_Fill)
					[
						SAssignNew(FoundGamesVerticalBox, SVerticalBox)
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Fill)
					]
				]
				+ SOverlay::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SAssignNew(WaitingTextBlock, STextBlock)
					.TextStyle(&Styles->WaitingTextStyle)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("lobby.waiting", "Waiting to load ..."))
				]
			]

			// Right Side - Buttons and TextEdit
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Fill)
			.Padding(30.f, 0, 0, 0)
			[
				SNew(SBox)
				.WidthOverride(300.f)
				.HAlign(HAlign_Fill)
				[
					SNew(SVerticalBox)

					// First Right side Vertical box - Session Name and Host button
					+ SVerticalBox::Slot()
					.VAlign(VAlign_Top)
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						[
							SNew(SBox)
							.HeightOverride(50.f)
							[
								SNew(SOverlay)

								+ SOverlay::Slot()
								.VAlign(VAlign_Fill)
								.HAlign(HAlign_Fill)
								[
									SNew(SImage)
									.ColorAndOpacity(Styles->EditableTextBackgroundColor)
									.Image(SlateBrushes[LastBrushSizeIndex - 2])
								]
								+ SOverlay::Slot()
								.VAlign(VAlign_Fill)
								.HAlign(HAlign_Fill)
								[
									SNew(SEditableText)
							
									.Style(&Styles->EditableTextStyle)
									.Justification(ETextJustify::Center)
									.CaretImage(SlateBrushes[LastBrushSizeIndex - 1])
									.Text(SessionName)
									.OnTextCommitted_Raw(this, &SLobbyWidget::OnSessionNameChanged)
								]
							]
						]

						+ SVerticalBox::Slot()
						.Padding(0, 30.f, 0, 0)
						[
							SNew(SBox)
							.HeightOverride(50.f)
							[
								SAssignNew(HostButton, SButton)
								.ContentPadding(FMargin(10.f, 0, 10.f, 0))
								.OnClicked_Raw(this, &SLobbyWidget::OnHostButtonClick)
								.Content()
								[
									SNew(STextBlock)
									.Justification(ETextJustify::Center)
									.TextStyle(&Styles->ButtonTextStyle)
									.Text(LOCTEXT("lobby.host", "Host"))
								]
							]
						]
					]

					// Second Right side Vertical box - Player Name, Join and Search buttons
					+ SVerticalBox::Slot()
					.VAlign(VAlign_Bottom)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						[
							SNew(SBox)
							.HeightOverride(50.f)
							[
								SNew(SOverlay)

								+ SOverlay::Slot()
								.VAlign(VAlign_Fill)
								.HAlign(HAlign_Fill)
								[
									SNew(SImage)
									.ColorAndOpacity(Styles->EditableTextBackgroundColor)
									.Image(SlateBrushes[LastBrushSizeIndex - 2])
								]
								+ SOverlay::Slot()
								.VAlign(VAlign_Fill)
								.HAlign(HAlign_Fill)
								[
									SNew(SEditableText)

									.Style(&Styles->EditableTextStyle)
									.Justification(ETextJustify::Center)
									.CaretImage(SlateBrushes[LastBrushSizeIndex - 1])
									.Text(PlayerName)
									.OnTextCommitted_Raw(this, &SLobbyWidget::OnPlayerNameChanged)
								]
							]
						]

						+ SVerticalBox::Slot()
						.Padding(0, 30.f, 0, 0)
						[
							SNew(SBox)
							.HeightOverride(50.f)
							[
								SAssignNew(JoinButton, SButton)
								.ContentPadding(FMargin(10.f, 0, 10.f, 0))
								.OnClicked_Raw(this, &SLobbyWidget::OnJoinButtonClick)
								.Content()
								[
									SNew(STextBlock)
									.Justification(ETextJustify::Center)
									.TextStyle(&Styles->ButtonTextStyle)
									.Text(LOCTEXT("lobby.join", "Join"))
								]
							]
						]

						+ SVerticalBox::Slot()
						.Padding(0, 30.f, 0, 0)
						[
							SNew(SBox)
							.HeightOverride(50.f)
							[
								SAssignNew(SearchButton, SButton)
								.ContentPadding(FMargin(10.f, 0, 10.f, 0))
								.OnClicked_Raw(this, &SLobbyWidget::OnSearchButtonClick)
								.Content()
								[
									SNew(STextBlock)
									.Justification(ETextJustify::Center)
									.TextStyle(&Styles->ButtonTextStyle)
									.Text(LOCTEXT("lobby.search", "Search"))
								]
							]
						]
					]
				]
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SLobbyWidget::OnHostButtonClick()
{
	if (SessionName.IsEmpty() || PlayerName.IsEmpty()) return FReply::Handled();

	HostButton.Get()->SetEnabled(false);
	JoinButton.Get()->SetEnabled(false);
	SearchButton.Get()->SetEnabled(false);

	LobbyGameMode->OnStartHosting(SessionName, PlayerName);

	return FReply::Handled();
}

FReply SLobbyWidget::OnSearchButtonClick()
{
	//SearchButton.Get()->SetEnabled(false);

	/*
	FoundGamesVerticalBox.Get()->AddSlot()
		[
			SNew(SButton)
		];
	*/

	LobbyGameMode->OnStartSearchingGames();
	return FReply::Handled();
}

FReply SLobbyWidget::OnJoinButtonClick()
{
	if (SessionName.IsEmpty() || PlayerName.IsEmpty()) return FReply::Handled();

	HostButton.Get()->SetEnabled(false);
	JoinButton.Get()->SetEnabled(false);
	SearchButton.Get()->SetEnabled(false);

	return FReply::Handled();
}

void SLobbyWidget::OnSessionNameChanged(const FText& InText, ETextCommit::Type CommitType)
{
	SessionName = InText;
}

void SLobbyWidget::OnPlayerNameChanged(const FText& InText, ETextCommit::Type CommitType)
{
	PlayerName = InText;
}

#undef LOCTEXT_NAMESPACE