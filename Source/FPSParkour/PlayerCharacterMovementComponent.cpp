// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacterMovementComponent.h"
#include "FPSParkourCharacter.h"




UPlayerCharacterMovementComponent::UPlayerCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

float UPlayerCharacterMovementComponent::GetMaxSpeed() const{
	float MaxSpeed = Super::GetMaxSpeed();

	const AFPSParkourCharacter* ShooterCharacterOwner = Cast<AFPSParkourCharacter>(PawnOwner);
	if (ShooterCharacterOwner)
	{
		if (ShooterCharacterOwner->IsCrouching())
		{
			MaxSpeed *= ShooterCharacterOwner->GetCrouchingSpeedModifier();
		}
		if (ShooterCharacterOwner->IsSprinting())
		{
			MaxSpeed *= ShooterCharacterOwner->GetRunningSpeedModifier();
		}
	}

	return MaxSpeed;
}
