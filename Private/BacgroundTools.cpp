// Copyright Epic Games, Inc. All Rights Reserved.

#include "BacgroundTools.h"
#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "Debug.h"
#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "SlateWidgets/AdvanceDeletionWidget.h"

#define LOCTEXT_NAMESPACE "FBacgroundToolsModule"

void FBacgroundToolsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	InitCBMenuExtention();

	RegisterAdvanceDeletionTab();
}

#pragma region ContentBrowserMenuExtention

void FBacgroundToolsModule::InitCBMenuExtention()
{
	FContentBrowserModule& ContentBrowserModule = 
	FModuleManager::LoadModuleChecked<FContentBrowserModule>(FName("ContentBrowser"));

	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBroswerModuleMenuExtenders = 
	ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	// delegate bind Case 1
	//FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	//CustomCBMenuDelegate.BindRaw(this, &FBacgroundToolsModule::CustomCBMenuExtender);

	//ContentBroswerModuleMenuExtenders.Add(CustomCBMenuDelegate);

	ContentBroswerModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::
		CreateRaw(this, &FBacgroundToolsModule::CustomCBMenuExtender));
}

TSharedRef<FExtender> FBacgroundToolsModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (SelectedPaths.Num() > 0)
	{

		MenuExtender->AddMenuExtension(FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FBacgroundToolsModule::AddCBMenuEntry));

		SelectedFolderPaths = SelectedPaths;
	}

	return  MenuExtender;
}


void FBacgroundToolsModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Unused Assets")), // title
		FText::FromString(TEXT("Safely delete all unused assets under folder")), // tooltip
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FBacgroundToolsModule::OnDeleteUnsuedAssetButtonClicked)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Empty Folders")), // title
		FText::FromString(TEXT("Safely delete all empty folders")), // tooltip
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FBacgroundToolsModule::OnDeleteEmptyFoldersButtonClicked)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Advanced Deletion")), // title
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")), // tooltip
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FBacgroundToolsModule::OnAdvancedDeletionButtonClicked)
	);
}

void FBacgroundToolsModule::OnDeleteUnsuedAssetButtonClicked()
{
	if (SelectedFolderPaths.Num() > 1)
	{
		Debug::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to one folder"));
		return;
	}

	/*Debug::PrintMessage(TEXT("Currently selected folder: ") + SelectedFolderPaths[0], FColor::Green);*/

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(SelectedFolderPaths[0]);

	if (AssetsPathNames.Num() == 0)
	{
		Debug::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset found under selected folder"));
		return;
	}


	EAppReturnType::Type ConfirmResult =
		Debug::ShowMsgDialog(
			EAppMsgType::YesNo,
			TEXT("A total of ") + FString::FromInt(AssetsPathNames.Num()) +
			TEXT(" assets need to be checked.\n Would you like to proceed?")
		);

	if (ConfirmResult == EAppReturnType::No) return;

	FixUpRedirectors();

	TArray<FAssetData> UnusedAssetsDataArray;

	for (const FString& AssetPathName : AssetsPathNames)
	{
		if (AssetPathName.Contains(TEXT("Developers")) || AssetPathName.Contains(TEXT("Collections")))
			continue;
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName))
			continue;

		TArray<FString> AssetReferancers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

		if (AssetReferancers.Num() == 0)
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsDataArray.Add(UnusedAssetData);
		}
	}

	if (UnusedAssetsDataArray.Num() > 0)
	{
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
	}
	else
	{
		Debug::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found under selected folder"));
	}
}

void FBacgroundToolsModule::OnDeleteEmptyFoldersButtonClicked()
{
	FixUpRedirectors();

	TArray<FString> FolderPathsArray;
	FolderPathsArray.Add(SelectedFolderPaths[0]); // 자기 자신 경로 추가
	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	
	TArray<FString> SubFolders;
	AssetRegistryModule.Get().GetSubPaths(SelectedFolderPaths[0], SubFolders, true);
	FolderPathsArray.Append(SubFolders); // 하위 폴더까지 모두 포함
	
	int32 Counter = 0;
	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathsArray;

	for (const FString& FolderPath : FolderPathsArray)
	{
		if (FolderPath.Contains(TEXT("Developers")) || FolderPath.Contains(TEXT("Collections")))
			continue;
		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath))
			continue;
		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));

			EmptyFoldersPathsArray.Add(FolderPath);
		}
	}

	if (EmptyFoldersPathsArray.Num() == 0)
	{
		Debug::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty folder found under selected folder"), false);
		return;
	}

	EAppReturnType::Type ConfirmResult = Debug::ShowMsgDialog(EAppMsgType::OkCancel,
		TEXT("Empty folders found in:\n") + EmptyFolderPathsNames + TEXT("\nWould you like to delete all?"), false);

	if (ConfirmResult == EAppReturnType::Cancel) return;

	for (const FString& EmptyFolderPath : EmptyFoldersPathsArray)
	{
		if (UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath))
			++Counter;
		else
			Debug::PrintMessage(TEXT("Failed to delete " + EmptyFolderPath), FColor::Red);
	}

	if (Counter > 0)
	{
		Debug::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(Counter) + TEXT("folders"));
	}

}

void FBacgroundToolsModule::OnAdvancedDeletionButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));
}

void FBacgroundToolsModule::FixUpRedirectors()
{
	IAssetRegistry& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace(FName("/Game"));
	Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());

	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);

	if (AssetList.Num() == 0) return;

	TArray<FString> ObjectPaths;
	for (const FAssetData& Asset : AssetList)
	{
		ObjectPaths.Add(Asset.GetObjectPathString());
	}

	TArray<UObject*> Objects;
	AssetViewUtils::FLoadAssetsSettings Settings;
	Settings.bFollowRedirectors = false;
	Settings.bAllowCancel = true;

	AssetViewUtils::ELoadAssetsResult Result = AssetViewUtils::LoadAssetsIfNeeded(ObjectPaths, Objects, Settings);

	if (Result != AssetViewUtils::ELoadAssetsResult::Cancelled)
	{
		// Transform Objects array to ObjectRedirectors array
		TArray<UObjectRedirector*> Redirectors;
		for (UObject* Object : Objects)
		{
			Redirectors.Add(CastChecked<UObjectRedirector>(Object));
		}

		// Load the asset tools module
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		AssetToolsModule.Get().FixupReferencers(Redirectors);
	}
}

#pragma endregion

#pragma region CustomEditorTab

void FBacgroundToolsModule::RegisterAdvanceDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this, &FBacgroundToolsModule::OnSpawnAdvanceDeletionTab))
		.SetDisplayName(FText::FromString(TEXT("Advance Deletion")));
}

TSharedRef<SDockTab> FBacgroundToolsModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return
		SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SAdvanceDeletionTab)
				.AssetsDataToStore(GetAllAssetData())
		];
}

TArray<TSharedPtr<FAssetData>> FBacgroundToolsModule::GetAllAssetData()
{
	TArray< TSharedPtr <FAssetData> > AvailableAssetsData;

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(SelectedFolderPaths[0]);

	for (const FString& AssetPathName : AssetsPathNames)
	{
		if (AssetPathName.Contains(TEXT("Developers")) || AssetPathName.Contains(TEXT("Collections")))
			continue;
		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName))
			continue;
		
		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);

		AvailableAssetsData.Add(MakeShared<FAssetData>(Data));
	}

	return AvailableAssetsData;
}

#pragma endregion

#pragma region ProccessForAdvanceDeletionTab

bool FBacgroundToolsModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);

	if (ObjectTools::DeleteAssets(AssetDataForDeletion) > 0)
	{
		return (true);
	}

	return (false);
}

#pragma endregion

void FBacgroundToolsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBacgroundToolsModule, BacgroundTools)