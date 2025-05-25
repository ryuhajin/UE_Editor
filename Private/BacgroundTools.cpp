// Copyright Epic Games, Inc. All Rights Reserved.

#include "BacgroundTools.h"
#include "ContentBrowserModule.h"

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

#pragma endregion

TSharedRef<FExtender> FBacgroundToolsModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (SelectedPaths.Num() > 0)
	{

		MenuExtender->AddMenuExtension(FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FBacgroundToolsModule::AddCBMenuEntry));
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
}

void FBacgroundToolsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBacgroundToolsModule, BacgroundTools)