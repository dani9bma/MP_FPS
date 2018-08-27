// Copyright 2017 UNAmedia. All Rights Reserved.

#pragma once

#include "SlateBasics.h"



class SRiggedSkeletonPicker : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRiggedSkeletonPicker)
		: _ReferenceRig(nullptr)
		{}

		SLATE_ARGUMENT(URig*, ReferenceRig)
		SLATE_ARGUMENT(FText, Title)
		SLATE_ARGUMENT(FText, Description)
	SLATE_END_ARGS()

public:
	SRiggedSkeletonPicker();
	void Construct(const FArguments& InArgs);

	USkeleton * GetSelectedSkeleton();

private:
	void OnAssetSelected(const FAssetData & AssetData);
	void OnAssetDoubleClicked(const FAssetData & AssetData);
	bool CanSelect() const;
	FReply OnSelect();
	FReply OnCancel();

	void CloseWindow();

private:
	// Track in AssetSkeleton the temporary selected asset,
	// only after the user confirms set SelectedSkeleton. So if
	// the widget is externally closed we don't report an un-selected
	// asset.
	USkeleton * ActiveSkeleton;
	USkeleton * SelectedSkeleton;
};
