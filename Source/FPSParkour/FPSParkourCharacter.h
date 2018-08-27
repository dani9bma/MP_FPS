// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Engine.h"
#include "GameFramework/Character.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "FPSParkourCharacter.generated.h"

class UInputComponent;

UENUM(BlueprintType)
enum class ETeamType : uint8 {
	CT UMETA(DisplayName = "Counter Terrorist"),
	T UMETA(DisplayName = "Terrorist")
};

UCLASS(config=Game)
class AFPSParkourCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh, Replicated)
	class USkeletalMeshComponent* FP_Gun;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh, Replicated)
	class USkeletalMeshComponent* FP_Pistol;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh, Replicated)
	class USkeletalMeshComponent* FP_Knife;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** Gun mesh: VR view (attached to the VR controller directly, no arm, just the actual gun) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* VR_Gun;

	/** Location on VR gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* VR_MuzzleLocation;

	/*Camera Boom*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComponent;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* L_MotionController;

public:
	AFPSParkourCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay();

	void Tick(float DeltaSeconds) override;

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StaticMeshAssets")
	USkeletalMesh* MeshTe;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StaticMeshAssets")
	USkeletalMesh* MeshCT;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AFPSParkourProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint32 bUsingMotionControllers : 1;

protected:

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	TouchData	TouchItem;

	/*Return the Aim Offsets*/
	UFUNCTION(BlueprintCallable)
	FRotator GetAimOffsets();

	/*Stars Sprint methods*/
	void Sprint();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSprint();

	void ServerSprint_Implementation();

	bool ServerSprint_Validate();

	/*Disable Sprint methods*/
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDisableSprint();

	void ServerDisableSprint_Implementation();

	bool ServerDisableSprint_Validate();

	void DisableSprint();
	/*Ends Sprint methods*/

	/*Stars Fire related methods*/
	void OffFire();

	/*SpawnEmitter methods*/
	void SpawnEmitter();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSpawnEmitter();

	void ServerSpawnEmitter_Implementation();

	bool ServerSpawnEmitter_Validate();
	/*End SpawnEmitter methods*/

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnEmitter();

	void MulticastSpawnEmitter_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerOnFire();

	void ServerOnFire_Implementation();

	bool ServerOnFire_Validate();

	/** Fires a projectile. */
	void OnFire();

	void AutomaticFire();

	void FireRaycast();

	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerFireRaycast();

	void ServerFireRaycast_Implementation();

	bool ServerFireRaycast_Validate();

	/*Play sound methods*/
	void PlaySound();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPlaySound();

	void ServerPlaySound_Implementation();

	bool ServerPlaySound_Validate();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlaySound();

	void MulticastPlaySound_Implementation();
	/*End Playsound methods*/
	/*End Fire related methods*/

	/*Starts crouch methods*/
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerOnCrouch();

	void ServerOnCrouch_Implementation();

	bool ServerOnCrouch_Validate();

	void OnCrouch();

	/*Disable Crouch methods*/
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerDisableCrouch();

	void ServerDisableCrouch_Implementation();

	bool ServerDisableCrouch_Validate();

	void DisableCrouch();
	/*Ends Crouch methods*/

	/*Starts Switch weapons methods*/
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerSwitchToPistol();

	void ServerSwitchToPistol_Implementation();

	bool ServerSwitchToPistol_Validate();

	void SwitchToPistol();

	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerSwitchToRifle();

	void ServerSwitchToRifle_Implementation();

	bool ServerSwitchToRifle_Validate();

	void SwitchToRifle();

	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerSwitchToKnife();

	void ServerSwitchToKnife_Implementation();

	bool ServerSwitchToKnife_Validate();

	void SwitchToKnife();
	/*Ends switch weapon method*/

	/*Starts Play animation methods*/
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPlayAnimation(UAnimMontage* AnimationToPlay);

	void ServerPlayAnimation_Implementation(UAnimMontage* AnimationToPlay);

	bool ServerPlayAnimation_Validate(UAnimMontage* AnimationToPlay);

	void PlayAnimation(UAnimMontage* AnimationToPlay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayAnimation(UAnimMontage* AnimationToPlay);

	void MulticastPlayAnimation_Implementation(UAnimMontage* AnimationToPlay);
	/*Ends play animation methods*/

	/*Replicates vars */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Recoil();

	void AddMainWidget();

	void RemoveMainWidget();

	void AddChooseTeamWidget();

	UFUNCTION(BlueprintCallable)
	void RemoveChooseTeamWidget();

	void CheckForReload();

	void Reload();

	void ReloadRifleAmmo();

	void ReloadPistolAmmo();

	void SetTeamMesh(ETeamType TeamType);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerSetTeamMesh(ETeamType TeamType);

	void ServerSetTeamMesh_Implementation(ETeamType TeamType);

	bool ServerSetTeamMesh_Validate(ETeamType TeamType);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MulticastSetTeamMesh(ETeamType TeamType);

	void MulticastSetTeamMesh_Implementation(ETeamType TeamType);
	
	bool MulticastSetTeamMesh_Validate(ETeamType TeamType);

	UFUNCTION(BlueprintCallable)
	void SetSpawn(ETeamType TeamType);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerSetSpawn(ETeamType TeamType);

	void ServerSetSpawn_Implementation(ETeamType TeamType);

	bool ServerSetSpawn_Validate(ETeamType TeamType);
public:
	bool IsSprinting() const;

	bool IsCrouching() const;

	float GetRunningSpeedModifier() const;

	float GetCrouchingSpeedModifier() const;
	
	int32 GetKillAmount() const;

	void Respawn();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRespawn();

	void ServerRespawn_Implementation();

	bool ServerRespawn_Validate();

	void SetDeath();

	ETeamType GetPlayerTeam() const;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetDeath();

	void ServerSetDeath_Implementation();

	bool ServerSetDeath_Validate();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetDeath();

	void MulticastSetDeath_Implementation();

	float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	void InflictDamage(AActor* HitActor, float DamageValue);

	bool bIsDead;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	int32 CTkills;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	int32 Tkills;

	UPROPERTY(Replicated)
	int32 KillAmount;

	UFUNCTION(BlueprintCallable)
	int32 GetHealth() const;

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentHealth() const;

	UFUNCTION(BlueprintCallable)
	int32 GetRifleAmmo() const;

	UFUNCTION(BlueprintCallable)
	int32 GetRifleCurrentAmmo() const;

	UFUNCTION(BlueprintCallable)
	int32 GetPistolAmmo() const;

	UFUNCTION(BlueprintCallable)
	int32 GetPistolCurrentAmmo() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	ETeamType PlayerTeam;

	bool bIsReloading;

	FTimerHandle FireHandle;

	FTimerHandle RespawnHandle;

	float FireRate;

	/*Changes the walk speed while running*/
	float RunningSpeedModifier;

	/*Changes the walk speed while crouching*/
	float CrouchingSpeedModifier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	int32 Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	int32 CurrentHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	int32 RifleAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	int32 RifleCurrentAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	int32 PistolAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	int32 PistolCurrentAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool bIsSprinting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool bIsShooting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool bIsCrouching;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool bIsAkEquipped;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool bIsPistolEquipped;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool bIsKnifeEquipped;

	UPROPERTY(EditDefaultsOnly, Category = "Animations")
	class UAnimMontage* SwitchToPistolAnimMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animations")
	class UAnimMontage* SwitchToRifleAnimMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animations")
	class UAnimMontage* SwitchToKnifeAnimMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animations")
	class UAnimMontage* KnifeAnimMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animations")
	class UAnimMontage* ReloadRifleAnimMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animations")
	class UAnimMontage* ReloadPistolAnimMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animations")
	class UAnimMontage* DeathAnimMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	class UParticleSystem* FireParticleSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player, HUD and UI")
	TSubclassOf<UUserWidget> MainUIClass;

	UUserWidget* MainWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player, HUD and UI")
	TSubclassOf<UUserWidget> ChooseTeamUIClass;

	UUserWidget* ChooseTeamWidget;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */

public:
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetCameraComponent() const { return CameraComponent; }

};

