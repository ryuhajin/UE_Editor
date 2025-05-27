// Copyright Epic Games, Inc. All Rights Reserved.

#include "BacgroundTools.h"
#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "Debug.h"

#define LOCTEXT_NAMESPACE "FBacgroundToolsModule"

void FBacgroundToolsModule::StartupModule()
{
	InitCBMenuExtention();
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
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
		FText::FromString(TEXT("Delete Unused Assets")),
		FText::FromString(TEXT("Safely delete all unused assets under folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FBacgroundToolsModule::OnDeleteUnsuedAssetButtonClicked)
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
			TEXT(" found.\n Would you like to proceed?")
		);

	if (ConfirmResult == EAppReturnType::No) return;

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
		else
		{
			Debug::PrintMessage(TEXT("Currently selected folder: ") + AssetReferancers[0], FColor::Green);
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

#pragma endregion

void FBacgroundToolsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBacgroundToolsModule, BacgroundTools)