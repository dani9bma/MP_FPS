// Copyright 2017 UNAmedia. All Rights Reserved.

#include "MixamoToolkitPrivatePCH.h"
#include "MixamoToolkitCommands.h"

#include "MixamoToolkitStyle.h"

#define LOCTEXT_NAMESPACE "FMixamoAnimationRetargetingModule"



FMixamoToolkitCommands::FMixamoToolkitCommands()
	: TCommands<FMixamoToolkitCommands>(
		TEXT("MixamoAnimationRetargeting"),		// Context name for fast lookup
		NSLOCTEXT(LOCTEXT_NAMESPACE, "MixamoAnimationRetargetingCommands", "Mixamo Animation Retargeting Plugin"),
		NAME_None,		// Parent
		FMixamoToolkitStyle::GetStyleSetName()		// Icon Style Set
	  )
{
}



void FMixamoToolkitCommands::RegisterCommands()
{
	//UI_COMMAND(OpenBatchConverterWindow, "Mixamo batch helper", "Open the batch helper for Mixamo assets", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(RetargetMixamoSkeleton, "Retarget Mixamo Skeleton Asset", "Retarget Mixamo Skeleton Assets", EUserInterfaceActionType::Button, FInputGesture());
}



#undef LOCTEXT_NAMESPACE
