// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "GameFramework/GameModeBase.h"
#include "FPSParkourGameMode.generated.h"

UCLASS(minimalapi)
class AFPSParkourGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFPSParkourGameMode();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int32 CTKills;

	UPROPERTY(Replicated)
	int32 TKills;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FText Winner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsEndGame;

	void Tick(float DeltaSeconds) override;

	void CheckEndGame();

	UFUNCTION(BlueprintCallable)
	int32 GetCTKills() const;

	UFUNCTION(BlueprintCallable)
	int32 GetTKills() const;
};



