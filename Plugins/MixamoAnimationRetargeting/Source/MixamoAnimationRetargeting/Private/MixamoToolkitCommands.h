// Copyright 2017 UNAmedia. All Rights Reserved.

#pragma once

#include "SlateBasics.h"



class FMixamoToolkitCommands : public TCommands<FMixamoToolkitCommands>
{
public:
	FMixamoToolkitCommands();

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	//TSharedPtr< FUICommandInfo > OpenBatchConverterWindow;
	TSharedPtr< FUICommandInfo > RetargetMixamoSkeleton;
};