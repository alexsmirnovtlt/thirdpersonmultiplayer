// Aleksandr Smirnov 2021


#include "Slate/SLobbyWidget.h"
#include "SlateOptMacros.h"

#include "Widgets/Input/SEditableText.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Images/SImage.h"

#include "Slate/Styles/LobbyMenuSlateWidgetStyle.h"
#include "General/Gamemodes/LobbyGameMode.h"
#include "Slate/SLobbyFoundGameInfoWidget.h"

#define LOCTEXT_NAMESPACE "LobbyWidget"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLobbyWidget::Construct(const FArguments& InArgs)
{
	if (!IsValid(InArgs._LobbyStyle)) return;
	if (!InArgs._SessionItemStyle.IsValid()) return;

	LobbyGameMode = InArgs._LobbyGameMode;
	SessionItemStyle = InArgs._SessionItemStyle;

	const FLobbyMenuSlateStyle& MainStyles = InArgs._LobbyStyle->WidgetStyle;

	SessionName = MainStyles.GetSessionNameDefaultText();
	PlayerName = MainStyles.GetPlayerNameDefaultText();

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
			.ColorAndOpacity(MainStyles.BackgroundColor)
			.Image(&MainStyles.BackgroundBrush)
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
					.TextStyle(&MainStyles.WaitingTextStyle)
					.Justification(ETextJustify::Center)
					.Visibility(EVisibility::Hidden)
					.Text(LOCTEXT("lobby.waiting", "Waiting to load ..."))
				]
				+ SOverlay::Slot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SAssignNew(NothingFoundTextBlock, STextBlock)
					.TextStyle(&MainStyles.WaitingTextStyle)
					.Justification(ETextJustify::Center)
					.Visibility(EVisibility::Hidden)
					.Text(LOCTEXT("lobby.empty", "Nothing was found"))
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
									.ColorAndOpacity(MainStyles.EditableTextBackgroundColor)
									.Image(&MainStyles.EditableTextBackgroundBrush)
								]
								+ SOverlay::Slot()
								.VAlign(VAlign_Fill)
								.HAlign(HAlign_Fill)
								[
									SNew(SEditableText)
							
									.Style(&MainStyles.EditableTextStyle)
									.Justification(ETextJustify::Center)
									.CaretImage(&MainStyles.EditableTextCaretBrush)
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
								.IsEnabled(true)
								.Content()
								[
									SNew(STextBlock)
									.Justification(ETextJustify::Center)
									.TextStyle(&MainStyles.ButtonTextStyle)
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
									.ColorAndOpacity(MainStyles.EditableTextBackgroundColor)
									.Image(&MainStyles.EditableTextBackgroundBrush)
								]
								+ SOverlay::Slot()
								.VAlign(VAlign_Fill)
								.HAlign(HAlign_Fill)
								[
									SNew(SEditableText)

									.Style(&MainStyles.EditableTextStyle)
									.Justification(ETextJustify::Center)
									.CaretImage(&MainStyles.EditableTextCaretBrush)
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
								.IsEnabled(false)
								.Content()
								[
									SNew(STextBlock)
									.Justification(ETextJustify::Center)
									.TextStyle(&MainStyles.ButtonTextStyle)
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
								.IsEnabled(true)
								.Content()
								[
									SNew(STextBlock)
									.Justification(ETextJustify::Center)
									.TextStyle(&MainStyles.ButtonTextStyle)
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

#undef LOCTEXT_NAMESPACE

FReply SLobbyWidget::OnHostButtonClick()
{
	if (SessionName.IsEmpty() || PlayerName.IsEmpty()) return FReply::Handled();

	SetButtonsEnabled(false, false, false);

	LobbyGameMode->OnStartHosting(SessionName);

	return FReply::Handled();
}

FReply SLobbyWidget::OnSearchButtonClick()
{
	WaitingTextBlock.Get()->SetVisibility(EVisibility::Visible);
	NothingFoundTextBlock.Get()->SetVisibility(EVisibility::Hidden);

	SetButtonsEnabled(false, false, false);

	LobbyGameMode->OnStartSearchingGames();

	return FReply::Handled();
}

FReply SLobbyWidget::OnJoinButtonClick()
{
	if (SessionName.IsEmpty() || PlayerName.IsEmpty()) return FReply::Handled();

	SetButtonsEnabled(false, false, false);

	LobbyGameMode.Get()->OnStartJoining(SessionName, ChosenSessionIndex);

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

void SLobbyWidget::SetButtonEnabled_Host(bool bIsEnabled)
{
	HostButton.Get()->SetEnabled(bIsEnabled);
}

void SLobbyWidget::SetButtonEnabled_Join(bool bIsEnabled)
{
	JoinButton.Get()->SetEnabled(bIsEnabled);
}

void SLobbyWidget::SetButtonEnabled_Search(bool bIsEnabled)
{
	SearchButton.Get()->SetEnabled(bIsEnabled);
}

void SLobbyWidget::SetButtonsEnabled(bool bHost, bool bJoin, bool bSearch)
{
	HostButton.Get()->SetEnabled(bHost);
	JoinButton.Get()->SetEnabled(bJoin);
	SearchButton.Get()->SetEnabled(bSearch);
}

void SLobbyWidget::ClearSessionsList()
{
	WaitingTextBlock.Get()->SetVisibility(EVisibility::Hidden);

	for (auto& Item : RunningGamesArray)
		FoundGamesVerticalBox.Get()->RemoveSlot(Item);

	RunningGamesArray.Empty();
}

void SLobbyWidget::ResetButtonState()
{
	WaitingTextBlock.Get()->SetVisibility(EVisibility::Hidden);
	NothingFoundTextBlock.Get()->SetVisibility(EVisibility::Hidden);
	SetButtonsEnabled(true, false, true);
}

void SLobbyWidget::DisplayNoSessionsFound()
{
	NothingFoundTextBlock.Get()->SetVisibility(EVisibility::Visible);
}

void SLobbyWidget::OnSessionItemSelected(int32 index)
{
	if (ChosenSessionIndex > -1) RunningGamesArray[ChosenSessionIndex]->Deselect();

	JoinButton.Get()->SetEnabled(true);

	ChosenSessionIndex = index;
}

const FString& SLobbyWidget::GetPlayerName() const
{
	return PlayerName.ToString();
}

void SLobbyWidget::AddFoundSession(FString& SessionNameStr, int32 CurrentPlayersCount, int32 MaxPlayersCount, int32 Index)
{
	if (!SessionItemStyle.IsValid()) return;

	auto ptr = SharedThis(this);

	TSharedRef<SLobbyFoundGameInfoWidget> NewItem = SNew(SLobbyFoundGameInfoWidget)
		.SlateStyle(SessionItemStyle.Get())
		.ParentLobbyWidget(ptr)
		.SessionNameStr(SessionNameStr)
		.CurrentPlayersCount(CurrentPlayersCount)
		.MaxPlayersCount(MaxPlayersCount)
		.OnlineSubsystemIndex(Index);

	FoundGamesVerticalBox.Get()->AddSlot() [ NewItem ]
	.AutoHeight()
	.HAlign(HAlign_Fill);

	RunningGamesArray.Add(NewItem);
}