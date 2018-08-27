// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSParkourPlayerController.generated.h"

/**
 * 
 */

UCLASS()
class FPSPARKOUR_API AFPSParkourPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	FTimerHandle TimerHandle_Respawn;

	void OnKilled();

	void Respawn();
};
