// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SAdvanceDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab) {}

	SLATE_ARGUMENT(TArray< TSharedPtr <FAssetData> >, AssetsDataToStore)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	TArray <TSharedPtr <FAssetData> > StoredAssetData;

	TSharedRef < SListView < TSharedPtr <FAssetData> > > ConstructAssetListView();

	TSharedPtr < SListView < TSharedPtr <FAssetData> > > ConstructedAssetListView;

	void RefreshAssetListView();

#pragma region RowWidgetForAssetListView

	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
		const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	void OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);

	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse);

	TSharedRef<SButton> ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetdata);

#pragma endregion


	TSharedRef<SButton> ConstructDeleteAllButton();
	TSharedRef<SButton> ConstructSelectAllButton();
	TSharedRef<SButton>ConstructDeselectAllButton();

	FReply OnDeleteAllButtonClicked();
	FReply OnSelectAllButtonClicked();
	FReply OnDeselectAllButtonClicked();

	TSharedRef<STextBlock> ConstructTextForTabButtons(const FString& TextContent);

	FSlateFontInfo GetEmbossedTextFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }
};
