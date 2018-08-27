// Copyright 2017 UNAmedia. All Rights Reserved.

#include "MixamoToolkitPrivatePCH.h"
#include "SMixamoToolkitWidget.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"	// FAssetPickerConfig
#include "Animation/Skeleton.h"
#include "Animation/Rig.h"



#define LOCTEXT_NAMESPACE "FMixamoAnimationRetargetingModule"



SRiggedSkeletonPicker::SRiggedSkeletonPicker()
	: ActiveSkeleton(nullptr),
	  SelectedSkeleton(nullptr)
{}



void SRiggedSkeletonPicker::Construct(const FArguments& InArgs)
{
	const URig * ReferenceRig = InArgs._ReferenceRig;

	checkf(ReferenceRig != nullptr, TEXT("A reference rig must be specified."));
	checkf(!InArgs._Title.IsEmpty(), TEXT("A title must be specified."));
	checkf(!InArgs._Description.IsEmpty(), TEXT("A description must be specified."));

	ActiveSkeleton = nullptr;
	SelectedSkeleton = nullptr;

	// Configure the Asset Picker.
	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.Filter.ClassNames.Add(USkeleton::StaticClass()->GetFName());
	AssetPickerConfig.Filter.bRecursiveClasses = true;
	if (ReferenceRig != nullptr)
	{
		AssetPickerConfig.Filter.TagsAndValues.Add(USkeleton::RigTag, ReferenceRig->GetFullName());
	}
	AssetPickerConfig.SelectionMode = ESelectionMode::Single;
	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SRiggedSkeletonPicker::OnAssetSelected);
	AssetPickerConfig.AssetShowWarningText = LOCTEXT("SRiggedSkeletonPicker_NoAssets", "No Skeleton asset with Humanoid rig found!");
	// Aesthetic settings.
	AssetPickerConfig.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &SRiggedSkeletonPicker::OnAssetDoubleClicked);
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::Column;
	AssetPickerConfig.bShowPathInColumnView = true;
	AssetPickerConfig.bShowTypeInColumnView = false;
	// Hide all asset registry columns by default (we only really want the name and path)
	TArray<UObject::FAssetRegistryTag> AssetRegistryTags;
	USkeleton::StaticClass()->GetDefaultObject()->GetAssetRegistryTags(AssetRegistryTags);
	for (UObject::FAssetRegistryTag & AssetRegistryTag : AssetRegistryTags)
	{
		AssetPickerConfig.HiddenColumnNames.Add(AssetRegistryTag.Name.ToString());
	}

	FContentBrowserModule & ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TSharedRef<SWidget> AssetPicker = ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig);

	ChildSlot[
		SNew(SVerticalBox)

			// Title text
			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
							[
								SNew(STextBlock)
									.Text(InArgs._Title)
									.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 16))
									.AutoWrapText(true)
							]
				]

			// Help description text
			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
						.Text(InArgs._Description)
						.AutoWrapText(true)
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(SSeparator)
				]

			// Asset picker.
			+ SVerticalBox::Slot()
				.MaxHeight(500)
				[
					AssetPicker
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)
				[
					SNew(SSeparator)
				]

			// Buttons
			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				[
					SNew(SUniformGridPanel)
						+ SUniformGridPanel::Slot(0, 0)
						[
							SNew(SButton)
								.HAlign(HAlign_Center)
								.Text(LOCTEXT("SRiggedSkeletonPicker_Ok", "Select"))
								.IsEnabled(this, &SRiggedSkeletonPicker::CanSelect)
								.OnClicked(this, &SRiggedSkeletonPicker::OnSelect)
						]
					+ SUniformGridPanel::Slot(1, 0)
						[
							SNew(SButton)
								.HAlign(HAlign_Center)
								.Text(LOCTEXT("SRiggedSkeletonPicker_Cancel", "Cancel"))
								.OnClicked(this, &SRiggedSkeletonPicker::OnCancel)
						]
				]
	];
}



USkeleton * SRiggedSkeletonPicker::GetSelectedSkeleton()
{
	return SelectedSkeleton;
}



void SRiggedSkeletonPicker::OnAssetSelected(const FAssetData & AssetData)
{
	ActiveSkeleton = Cast<USkeleton>(AssetData.GetAsset());
}



void SRiggedSkeletonPicker::OnAssetDoubleClicked(const FAssetData & AssetData)
{
	OnAssetSelected(AssetData);
	OnSelect();
}



bool SRiggedSkeletonPicker::CanSelect() const
{
	return ActiveSkeleton != nullptr;
}



FReply SRiggedSkeletonPicker::OnSelect()
{
	SelectedSkeleton = ActiveSkeleton;
	CloseWindow();
	return FReply::Handled();
}



FReply SRiggedSkeletonPicker::OnCancel()
{
	SelectedSkeleton = nullptr;
	CloseWindow();
	return FReply::Handled();
}



void SRiggedSkeletonPicker::CloseWindow()
{
	TSharedPtr<SWindow> window = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (window.IsValid())
	{
		window->RequestDestroyWindow();
	}
}



#undef LOCTEXT_NAMESPACE
