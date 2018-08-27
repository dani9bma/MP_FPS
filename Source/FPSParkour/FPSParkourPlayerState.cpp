// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSParkourPlayerState.h"
#include "UnrealNetwork.h"

void AFPSParkourPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSParkourPlayerState, bTeamB);
}




