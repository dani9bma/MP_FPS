// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "FPSParkourPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class FPSPARKOUR_API AFPSParkourPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Defaults")
	bool bTeamCT;
	
	
};
