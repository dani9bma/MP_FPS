// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "FPSParkourSpawnPoint.generated.h"

/**
 * 
 */
UCLASS()
class FPSPARKOUR_API AFPSParkourSpawnPoint : public ATargetPoint
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Defaults")
		bool bTeamCT;
	
	
};
