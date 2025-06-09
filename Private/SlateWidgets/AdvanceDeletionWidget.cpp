// Fill out your copyright notice in the Description page of Project Settings.

#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "SlateBasics.h"
#include "BacgroundTools.h"
#include "Debug.h"

void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	StoredAssetData = InArgs._AssetsDataToStore;

	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleTextFont.Size = 30;

	ChildSlot
		[
			// Main vertical box
			SNew(SVerticalBox)
			
				// First vertical slot for title text
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Advance Deletion")))
				.Font(TitleTextFont)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FColor::White)
			]

			//Second Slot for drop down list
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
			]

			//Third slot for the asset list
			+SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
			[
				SNew(SScrollBox)
				
				+SScrollBox::Slot()
				[
					ConstructAssetListView()
				]
				
			]

			//Foutrh slot for 3 buttons
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				// button 1
				+SHorizontalBox::Slot()
				.FillWidth(10.f)
				.Padding(5.f)
				[
					ConstructDeleteAllButton()
				]
				// button 2
				+ SHorizontalBox::Slot()
				.FillWidth(10.f)
				.Padding(5.f)
				[
					ConstructSelectAllButton()
				]
				// button 3
				+ SHorizontalBox::Slot()
				.FillWidth(10.f)
				.Padding(5.f)
				[
					ConstructDeselectAllButton()
				]
			]
		];
}

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvanceDeletionTab::ConstructAssetListView()
{
	ConstructedAssetListView = SNew(SListView< TSharedPtr <FAssetData> >)
		.ItemHeight(24.f)
		.ListItemsSource(&StoredAssetData)
		.OnGenerateRow(this, &SAdvanceDeletionTab::OnGenerateRowForList);

	return ConstructedAssetListView.ToSharedRef();
}

#pragma region RowWidgetForAssetListView

TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!AssetDataToDisplay.IsValid()) return SNew(STableRow< TSharedPtr <FAssetData> >, OwnerTable);

	// UE 5 이상부터 AssetClass -> AssetClassPath로 변경 AssetClassPath는 경로. 그 안에 겟네임 해야 클래스명
	const FString DisplayAssetClass = AssetDataToDisplay->AssetClassPath.GetAssetName().ToString();
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	FSlateFontInfo AssetClassFont = GetEmbossedTextFont();
	AssetClassFont.Size = 10;

	FSlateFontInfo AssetNameFont = GetEmbossedTextFont();
	AssetNameFont.Size = 15;


	TSharedRef < STableRow < TSharedPtr <FAssetData> > > ListViewRowWidget =
		SNew(STableRow < TSharedPtr<FAssetData> >, OwnerTable)
		[
			SNew(SHorizontalBox)

				// first : check box
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Bottom)
				.FillWidth(.02f)
				[
					ConstructCheckBox(AssetDataToDisplay)
				]

				// second : asset class name
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Fill)
				.FillWidth(.2f)
				[
					ConstructTextForRowWidget(DisplayAssetClass, AssetClassFont)
				]


				// third : display asset name
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Fill)
				[
					ConstructTextForRowWidget(DisplayAssetName, AssetNameFont)
				]

				//fourth : buttom
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Fill)
				[
					ConstructButtonForRowWidget(AssetDataToDisplay)
				]
		];

	return ListViewRowWidget;
}


TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox = SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this, &SAdvanceDeletionTab::OnCheckBoxStateChanged, AssetDataToDisplay)
		.Visibility(EVisibility::Visible);

	return ConstructedCheckBox;
}

void SAdvanceDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		Debug::PrintMessage(AssetData->AssetName.ToString() + TEXT(" is unchecked"), FColor::Red);
		break;
	case ECheckBoxState::Checked:
		Debug::PrintMessage(AssetData->AssetName.ToString() + TEXT(" is checked"), FColor::Green);
		break;
	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructButton = SNew(SButton)
		.Text(FText::FromString(TEXT("Delete")))
		.OnClicked(this, &SAdvanceDeletionTab::OnDeleteButtonClicked, AssetDataToDisplay);

	return ConstructButton;
}

FReply SAdvanceDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetdata)
{
	FBacgroundToolsModule& BacgroundToolsModule = 
		FModuleManager::LoadModuleChecked<FBacgroundToolsModule>(TEXT("BacgroundTools"));

	const bool bAssetDeleted = BacgroundToolsModule.DeleteSingleAssetForAssetList(*ClickedAssetdata.Get());

	if(bAssetDeleted)
	{
		//Updationg the list source items
		if (StoredAssetData.Contains(ClickedAssetdata))
		{
			StoredAssetData.Remove(ClickedAssetdata);
		}

		// refresh the list
		RefreshAssetListView();
	}

	return FReply::Handled();
}


TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> ConstructTextBlock = SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(FontToUse)
		.ColorAndOpacity(FColor::White);

	return ConstructTextBlock;
}

#pragma endregion

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvanceDeletionTab::OnDeleteAllButtonClicked);

	DeleteAllButton->SetContent(ConstructTextForTabButtons(TEXT("Delete All")));

	return  DeleteAllButton;
}


TSharedRef<SButton> SAdvanceDeletionTab::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvanceDeletionTab::OnSelectAllButtonClicked);

	SelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Select All")));

	return  SelectAllButton;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeselectAllButton()
{
	TSharedRef<SButton> DeselectAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvanceDeletionTab::OnDeselectAllButtonClicked);

	DeselectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Deselect All")));

	return DeselectAllButton;
}

FReply SAdvanceDeletionTab::OnDeleteAllButtonClicked()
{
	Debug::PrintMessage(TEXT("Delete All Button Clicked"), FColor::Cyan);
	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnSelectAllButtonClicked()
{
	Debug::PrintMessage(TEXT("Select All Button Clicked"), FColor::Cyan);
	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnDeselectAllButtonClicked()
{
	Debug::PrintMessage(TEXT("Deselect All Button Clicked"), FColor::Cyan);
	return FReply::Handled();
}


TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForTabButtons(const FString& TextContent)
{
	FSlateFontInfo ButtonTextFont = GetEmbossedTextFont();
	ButtonTextFont.Size = 15;

	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(ButtonTextFont)
		.Justification(ETextJustify::Center);

	return ConstructedTextBlock;
}

void SAdvanceDeletionTab::RefreshAssetListView()
{
	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}
