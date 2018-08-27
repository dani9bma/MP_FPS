// Copyright 2017 UNAmedia. All Rights Reserved.

#pragma once

#include "ModuleManager.h"


DECLARE_LOG_CATEGORY_EXTERN(LogMixamoToolkit, Warning, All)



class FMixamoAnimationRetargetingModule :
	public IModuleInterface,
	public TSharedFromThis<FMixamoAnimationRetargetingModule>
{
public:
	static
	FMixamoAnimationRetargetingModule & Get();

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedRef<class FMixamoSkeletonRetargeter> GetMixamoSkeletonRetargeter();

private:
	TSharedPtr<class FMixamoSkeletonRetargeter> MixamoSkeletonRetargeter;
	TSharedPtr<class FMixamoToolkitEditorIntegration> EditorIntegration;
};