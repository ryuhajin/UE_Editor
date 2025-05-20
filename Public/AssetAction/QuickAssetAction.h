// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
//#include "AssetToolsModule.h"
//#include "AssetViewUtils.h"
//#include "AssetRegistry/AssetRegistryModule.h"
#include "QuickAssetAction.generated.h"

/**
 * 
 */
UCLASS()
class BACGROUNDTOOLS_API UQuickAssetAction : public UAssetActionUtility
{
	GENERATED_BODY()

public:
	UFUNCTION(CallInEditor)
	void DuplicateAssets(int32 NumOfDuplicates);
};
