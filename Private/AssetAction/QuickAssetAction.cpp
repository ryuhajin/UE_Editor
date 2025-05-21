// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetAction/QuickAssetAction.h"
#include "Debug.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "Misc/MessageDialog.h"

void UQuickAssetAction::DuplicateAssets(int32 NumOfDuplicates)
{
	if (NumOfDuplicates <= 0)
	{
		/*PrintMessage(TEXT("Please enter a VALID number"), FColor::Red);*/
		ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a VALID number"));
		return;
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		for (int32 i = 0; i < NumOfDuplicates; i++)
		{
			const FString SourceAssetPath = SelectedAssetData.ObjectPath.ToString();
			const FString NewDuplicateAssetName = SelectedAssetData.AssetName.ToString() +
				TEXT("_") + FString::FromInt(i);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(), NewDuplicateAssetName);

			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				++Counter;
			}
		}
	}

	if (Counter > 0)
	{
		/*PrintMessage(TEXT("Successfully Duplicated " + FString::FromInt(Counter) + " files"), FColor::Green);*/
		ShowNotifyInfo(TEXT("Successfully Duplicated " + FString::FromInt(Counter) + " files"));
	}
}

void UQuickAssetAction::AddPrefixes()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for (UObject* SelectedObject:SelectedObjects)
	{
		if (!SelectedObject) continue;

		FString* PrefixFound = this->PrefixMap.Find(SelectedObject->GetClass());

		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			PrintMessage(TEXT("Failed to find prefix for class ") + SelectedObject->GetClass()->GetName(),
				FColor::Red);
			continue;
		}

		FString OldName = SelectedObject->GetName();

		if (OldName.StartsWith(*PrefixFound))
		{
			PrintMessage(OldName + TEXT(" already has prefix added"), FColor::Red);
			continue;
		}

		const FString NewNameWithPrefix = *PrefixFound + OldName;

		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);

		++Counter;
	}

	ShowNotifyInfo(TEXT("Successfully renamed " + FString::FromInt(Counter) + " assets"));
}
