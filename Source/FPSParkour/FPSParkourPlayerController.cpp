// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSParkourPlayerController.h"
#include "FPSParkour.h"

void AFPSParkourPlayerController::OnKilled(){
	UnPossess();
	GetWorldTimerManager().SetTimer(TimerHandle_Respawn, this, &AFPSParkourPlayerController::Respawn, 5.f);
}

void AFPSParkourPlayerController::Respawn(){
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (GameMode) {
		APawn* NewPawn = GameMode->SpawnDefaultPawnFor(this, GameMode->ChoosePlayerStart(this));
		Possess(NewPawn);
	}
}

