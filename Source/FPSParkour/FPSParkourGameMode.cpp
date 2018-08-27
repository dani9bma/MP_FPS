// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSParkourGameMode.h"
#include "FPSParkourHUD.h"
#include "FPSParkourCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "FPSParkourPlayerStart.h"
#include "FPSParkourPlayerState.h"
#include "FPSParkourPlayerController.h"
#include "UnrealNetwork.h"

AFPSParkourGameMode::AFPSParkourGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFPSParkourHUD::StaticClass();

	PlayerControllerClass = AFPSParkourPlayerController::StaticClass();
}

void AFPSParkourGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSParkourGameMode, CTKills);
	DOREPLIFETIME(AFPSParkourGameMode, TKills);
}

void AFPSParkourGameMode::Tick(float DeltaSeconds){
	//Get All kills
	for (TActorIterator<AFPSParkourCharacter> It(GetWorld()); It; ++It) {
		AFPSParkourCharacter* Character = *It;
		if (Character) {
			if (Character->GetPlayerTeam() == ETeamType::CT) {
				CTKills += Character->GetKillAmount();
			}
			else if (Character->GetPlayerTeam() == ETeamType::T) {
				TKills += Character->GetKillAmount();
			}
		}
	}
}

void AFPSParkourGameMode::CheckEndGame(){
	if (CTKills == 100) {
		Winner = FText::FromString("Counter Terrorist");
	}
	else if(TKills == 100){
		Winner = FText::FromString("Terrorist");
	}
	bIsEndGame = true;
}

int32 AFPSParkourGameMode::GetCTKills() const{
	return CTKills;
}

int32 AFPSParkourGameMode::GetTKills() const{
	return TKills;
}

