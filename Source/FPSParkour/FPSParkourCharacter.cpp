// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSParkourCharacter.h"
#include "FPSParkourProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "MotionControllerComponent.h"
#include "PlayerCharacterMovementComponent.h"
#include "FPSParkourPlayerController.h"
#include "FPSParkourGameMode.h"
#include "FPSParkourSpawnPoint.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

#define MAX_HEALTH 100
#define MAX_CURRENT_RIFLE_AMMO 30
#define MAX_CURRENT_PISTOL_AMMO 12
#define MAX_RIFLE_AMMO 90
#define MAX_PISTOL_AMMO 36

//////////////////////////////////////////////////////////////////////////
// AFPSParkourCharacter

AFPSParkourCharacter::AFPSParkourCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPlayerCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	bReplicates = true;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));
	CameraComponent->SetupAttachment(CameraBoom);
	CameraBoom->bUsePawnControlRotation = false;

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(GetMesh(), TEXT("GripPoint"));
	FP_Gun->SetIsReplicated(true);


	FP_Pistol = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Pistol"));
	FP_Pistol->SetOnlyOwnerSee(false);			
	FP_Pistol->bCastDynamicShadow = false;
	FP_Pistol->CastShadow = false;
	FP_Pistol->SetupAttachment(GetMesh(), TEXT("PistolSocket"));
	FP_Pistol->SetIsReplicated(true);

	FP_Knife = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Knife"));
	FP_Knife->SetOnlyOwnerSee(false);			
	FP_Knife->bCastDynamicShadow = false;
	FP_Knife->CastShadow = false;
	FP_Knife->SetupAttachment(GetMesh(), TEXT("KnifeSocket"));
	FP_Knife->SetIsReplicated(true);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	/*Create T Mesh*/
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshTobj(TEXT("SkeletalMesh'/Game/Soldiers/TAL/TalSoldier2.TalSoldier2'"));
	MeshTe = MeshTobj.Object;
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshCTobj(TEXT("SkeletalMesh'/Game/Soldiers/USA/usasoldier_rifleman1.usasoldier_rifleman1'"));
	MeshCT = MeshCTobj.Object;

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for GetMesh(), FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.
}

void AFPSParkourCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSParkourCharacter, bIsSprinting);
	DOREPLIFETIME(AFPSParkourCharacter, bIsCrouching);
	DOREPLIFETIME(AFPSParkourCharacter, bIsShooting);
	DOREPLIFETIME(AFPSParkourCharacter, bIsAkEquipped);
	DOREPLIFETIME(AFPSParkourCharacter, bIsPistolEquipped);
	DOREPLIFETIME(AFPSParkourCharacter, bIsKnifeEquipped);
	DOREPLIFETIME(AFPSParkourCharacter, RifleAmmo);
	DOREPLIFETIME(AFPSParkourCharacter, RifleCurrentAmmo);
	DOREPLIFETIME(AFPSParkourCharacter, PistolAmmo);
	DOREPLIFETIME(AFPSParkourCharacter, PistolCurrentAmmo);
	DOREPLIFETIME(AFPSParkourCharacter, Health);
	DOREPLIFETIME(AFPSParkourCharacter, CurrentHealth);
	DOREPLIFETIME(AFPSParkourCharacter, PlayerTeam);
	DOREPLIFETIME(AFPSParkourCharacter, CTkills);
	DOREPLIFETIME(AFPSParkourCharacter, Tkills);
}

float AFPSParkourCharacter::GetRunningSpeedModifier() const{
	return RunningSpeedModifier;
}

float AFPSParkourCharacter::GetCrouchingSpeedModifier() const{
	return CrouchingSpeedModifier;
}

int32 AFPSParkourCharacter::GetKillAmount() const{
	return KillAmount;
}

void AFPSParkourCharacter::Respawn(){

	ServerSetSpawn(PlayerTeam);
	CurrentHealth	  = MAX_HEALTH;
	RifleCurrentAmmo  = MAX_CURRENT_RIFLE_AMMO;
	PistolCurrentAmmo = MAX_CURRENT_PISTOL_AMMO;
	RifleAmmo		  = MAX_RIFLE_AMMO;
	PistolAmmo		  = MAX_PISTOL_AMMO;
	GetWorld()->GetTimerManager().ClearTimer(RespawnHandle);
	EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	bIsDead = false;
}

void AFPSParkourCharacter::ServerRespawn_Implementation(){
	Respawn();
}

bool AFPSParkourCharacter::ServerRespawn_Validate(){
	return true;
}

ETeamType AFPSParkourCharacter::GetPlayerTeam() const {
	return PlayerTeam;
}

void AFPSParkourCharacter::SetDeath() {
	ServerPlayAnimation(DeathAnimMontage);
	bIsDead = true;

	if (Role < ROLE_Authority) {
		ServerSetDeath();
	}
}

void AFPSParkourCharacter::ServerSetDeath_Implementation(){
	SetDeath();
}

bool AFPSParkourCharacter::ServerSetDeath_Validate(){
	return true;
}

void AFPSParkourCharacter::MulticastSetDeath_Implementation(){
	SetDeath();
}

float AFPSParkourCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser){
	// Call the base class - this will tell us how much damage to apply  
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		CurrentHealth -= ActualDamage;
		// If the damage depletes our health set our lifespan to zero - which will destroy the actor
		if (CurrentHealth <= 0.f && !bIsDead)
		{
			ServerSetDeath();
			GetWorld()->GetTimerManager().SetTimer(RespawnHandle, this, &AFPSParkourCharacter::ServerRespawn, 3.f, false, -1.f);
			DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		}
	}

	return ActualDamage;
}

void AFPSParkourCharacter::InflictDamage(AActor* HitActor, float DamageValue){
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController != nullptr)
	{
		AActor* ImpactActor = HitActor;
		if ((ImpactActor != nullptr) && (ImpactActor != this))
		{
			// Create a damage event  
			TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
			FDamageEvent DamageEvent(ValidDamageTypeClass);

			const float DamageAmount = DamageValue;
			ImpactActor->TakeDamage(DamageAmount, DamageEvent, PlayerController, this);

			AFPSParkourCharacter* ImpactCharacter = Cast<AFPSParkourCharacter>(ImpactActor);
			if (ImpactCharacter) {
				if (ImpactCharacter->GetCurrentHealth() <= 0) {
					if (PlayerTeam == ETeamType::CT) {
						CTkills += 1;
					}
					else if (PlayerTeam == ETeamType::T) {
						Tkills += 1;
					}
				}

			}
		}

		
	}

}

int32 AFPSParkourCharacter::GetHealth() const{
	return Health;
}

int32 AFPSParkourCharacter::GetCurrentHealth() const{
	return CurrentHealth;
}

int32 AFPSParkourCharacter::GetRifleAmmo() const{
	return RifleAmmo;
}

int32 AFPSParkourCharacter::GetRifleCurrentAmmo() const{
	return RifleCurrentAmmo;
}

int32 AFPSParkourCharacter::GetPistolAmmo() const{
	return PistolAmmo;
}

int32 AFPSParkourCharacter::GetPistolCurrentAmmo() const{
	return PistolCurrentAmmo;
}

bool AFPSParkourCharacter::IsSprinting() const{
	return bIsSprinting;
}

bool AFPSParkourCharacter::IsCrouching() const{
	return bIsCrouching;
}

void AFPSParkourCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	FP_Pistol->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("PistolSocket"));
	FP_Knife->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("KnifeSocket"));

	bIsAkEquipped = true;

	GetMesh()->SetHiddenInGame(false, true);
	GetMesh()->SetIsReplicated(true);

	/*Modifier Vars*/
	RunningSpeedModifier = 1.5f;
	CrouchingSpeedModifier = 0.5f;
	FireRate = 0.12f;
	CurrentHealth     = MAX_HEALTH;
	Health			  = MAX_HEALTH;
	PistolAmmo		  = MAX_CURRENT_RIFLE_AMMO;
	PistolCurrentAmmo = MAX_CURRENT_PISTOL_AMMO;
	RifleAmmo		  = MAX_RIFLE_AMMO;
	RifleCurrentAmmo  = MAX_PISTOL_AMMO;

	AddMainWidget();
	AddChooseTeamWidget();
}

void AFPSParkourCharacter::Tick(float DeltaSeconds){
	AFPSParkourGameMode* GameMode = Cast<AFPSParkourGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode) {
		if (GameMode->bIsEndGame) {
			DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFPSParkourCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AFPSParkourCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AFPSParkourCharacter::DisableSprint);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AFPSParkourCharacter::OnCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AFPSParkourCharacter::DisableCrouch);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSParkourCharacter::OnFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPSParkourCharacter::OffFire);
	PlayerInputComponent->BindAction("SwitchToPistol", IE_Pressed, this, &AFPSParkourCharacter::SwitchToPistol);
	PlayerInputComponent->BindAction("SwitchToRifle", IE_Pressed, this, &AFPSParkourCharacter::SwitchToRifle);
	PlayerInputComponent->BindAction("SwitchToKnife", IE_Pressed, this, &AFPSParkourCharacter::SwitchToKnife);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AFPSParkourCharacter::Reload);


	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSParkourCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSParkourCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFPSParkourCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFPSParkourCharacter::LookUpAtRate);
}

void AFPSParkourCharacter::Recoil() {
	if (bIsAkEquipped) {
		AddControllerPitchInput(-0.5f);
	}
	else if (bIsPistolEquipped) {
		AddControllerPitchInput(-0.15f);
	}
}

void AFPSParkourCharacter::AddMainWidget(){
	if (MainUIClass) {
		MainWidget = CreateWidget<UUserWidget>(GetWorld(), MainUIClass);
		MainWidget->AddToViewport();
	}
}

void AFPSParkourCharacter::RemoveMainWidget(){
	MainWidget->RemoveFromViewport();
}

void AFPSParkourCharacter::AddChooseTeamWidget(){
	if (ChooseTeamUIClass) {
		ChooseTeamWidget = CreateWidget<UUserWidget>(GetWorld(), ChooseTeamUIClass);
		ChooseTeamWidget->AddToViewport();
		DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->bShowMouseCursor = true;
	}
}

void AFPSParkourCharacter::RemoveChooseTeamWidget(){
	ChooseTeamWidget->RemoveFromParent();
	EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->bShowMouseCursor = false;
}

void AFPSParkourCharacter::CheckForReload(){
	if (bIsAkEquipped) {
		if (RifleCurrentAmmo == 0) {
			Reload();
		}
	}
	else if (bIsPistolEquipped) {
		if (PistolCurrentAmmo == 0) {
			Reload();
		}
	}

}

void AFPSParkourCharacter::Reload(){
	if (bIsAkEquipped) {
		bIsReloading = true;
		ServerPlayAnimation(ReloadRifleAnimMontage);
		FTimerHandle ReloadHandle;
		GetWorld()->GetTimerManager().SetTimer(ReloadHandle, this, &AFPSParkourCharacter::ReloadRifleAmmo, 3.8f, false, -1.f);
	}
	else if (bIsPistolEquipped){
		bIsReloading = true;
		ServerPlayAnimation(ReloadPistolAnimMontage);
		FTimerHandle ReloadHandle;
		GetWorld()->GetTimerManager().SetTimer(ReloadHandle, this, &AFPSParkourCharacter::ReloadPistolAmmo, 3.7f, false, -1.f);
	}
}

void AFPSParkourCharacter::ReloadRifleAmmo(){
	int32 AmmoLeft = 30 - RifleCurrentAmmo;
	RifleAmmo -= AmmoLeft;
	RifleCurrentAmmo = 30;
	bIsReloading = false;
}

void AFPSParkourCharacter::ReloadPistolAmmo(){
	int32 AmmoLeft = 12 - PistolCurrentAmmo;
	PistolAmmo -= AmmoLeft;
	PistolCurrentAmmo = 12;
	bIsReloading = false;
}

void AFPSParkourCharacter::SetTeamMesh(ETeamType TeamType){
	if (TeamType == ETeamType::CT) {
		GetMesh()->SetSkeletalMesh(MeshCT);
	}
	else if(TeamType == ETeamType::T){
		GetMesh()->SetSkeletalMesh(MeshTe);
	}
}

void AFPSParkourCharacter::ServerSetTeamMesh_Implementation(ETeamType TeamType){
	MulticastSetTeamMesh(TeamType);
}

bool AFPSParkourCharacter::ServerSetTeamMesh_Validate(ETeamType TeamType){
	return true;
}

void AFPSParkourCharacter::MulticastSetTeamMesh_Implementation(ETeamType TeamType){
	SetTeamMesh(TeamType);
}

bool AFPSParkourCharacter::MulticastSetTeamMesh_Validate(ETeamType TeamType){
	return true;
}

void AFPSParkourCharacter::SetSpawn(ETeamType TeamType){
	for (TActorIterator<AFPSParkourSpawnPoint> It(GetWorld()); It; ++It) {
		AFPSParkourSpawnPoint* TestSpawn = *It;
		if (TeamType == ETeamType::CT && TestSpawn->bTeamCT) {
			SetActorLocation(TestSpawn->GetActorLocation());
			SetActorRotation(TestSpawn->GetActorRotation());
		}
		else if (TeamType == ETeamType::T && !(TestSpawn->bTeamCT)) {
			SetActorLocation(TestSpawn->GetActorLocation());
			SetActorRotation(TestSpawn->GetActorRotation());
		}
	}

	if (Role < ROLE_Authority) {
		ServerSetSpawn(TeamType);
	}
}

void AFPSParkourCharacter::ServerSetSpawn_Implementation(ETeamType TeamType){
	SetSpawn(TeamType);
}

bool AFPSParkourCharacter::ServerSetSpawn_Validate(ETeamType TeamType){
	return true;
}

void AFPSParkourCharacter::OnFire(){
	/*Checks if the pistol is equipped calls the normal Fire Function if it isnt calls the Fire function with a timer to make it automatic*/
	if (bIsPistolEquipped) {
		AutomaticFire();
	}
	else if(bIsAkEquipped){
		AutomaticFire();
		GetWorld()->GetTimerManager().SetTimer(FireHandle, this, &AFPSParkourCharacter::AutomaticFire, FireRate, true, -1.f);
	}
	else {
		AutomaticFire();
		GetWorld()->GetTimerManager().SetTimer(FireHandle, this, &AFPSParkourCharacter::AutomaticFire, 1.5f, true, -1.f);
	}
	
	
}

void AFPSParkourCharacter::AutomaticFire(){
	/*Executes this if knife is not equipped*/
	if (!bIsKnifeEquipped) {
		if (!bIsSprinting && !bIsReloading) {
			// try and fire a projectile
			if (ProjectileClass != NULL)
			{
				UWorld* const World = GetWorld();
				if (World != NULL)
				{
					const FRotator SpawnRotation = GetControlRotation();
					// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
					const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

					//Set Spawn Collision Handling Override
					FActorSpawnParameters ActorSpawnParams;
					ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

					// spawn the projectile at the muzzle
					World->SpawnActor<AFPSParkourProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);


					/*Check What weapon is equipped and shots from the socket of that weapon*/
					ServerSpawnEmitter();

					//Raycast
					ServerFireRaycast();

					/*Removes ammo start*/
					if (bIsAkEquipped) {
						RifleCurrentAmmo--;
					}
					else if (bIsPistolEquipped) {
						PistolCurrentAmmo--;
					}
					/*Removes ammo end*/

					Recoil();
					CheckForReload();
					bIsShooting = true;
				}
			}
			ServerPlaySound();
		}
	}
	/*Executes this if Knife is equipped*/
	else {
		ServerFireRaycast();

		ServerPlayAnimation(KnifeAnimMontage);
	}
}

void AFPSParkourCharacter::FireRaycast(){
	if (bIsKnifeEquipped) {
		/*Raycasts to see if there is any enemy on the crosshair*/
		FHitResult HitResult;
		FVector Start = FVector(CameraComponent->GetComponentLocation().X, CameraComponent->GetComponentLocation().Y, CameraComponent->GetComponentLocation().Z);
		FVector End = (CameraComponent->GetForwardVector() * 300.f) + Start;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params)) {
			if (HitResult.Actor->ActorHasTag("Hittable")) {
				InflictDamage(HitResult.GetActor(), 50.f);
				UE_LOG(LogTemp, Warning, TEXT("Hit"));
			}
		}
	}
	else {
		/*Raycasts to see if there is any enemy on the crosshair*/
		FHitResult HitResult;
		FVector Start = FVector(CameraComponent->GetComponentLocation().X, CameraComponent->GetComponentLocation().Y, CameraComponent->GetComponentLocation().Z);
		FVector End = (CameraComponent->GetForwardVector() * 10000.f) + Start;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params)) {
			if (HitResult.Actor->ActorHasTag("Hittable")) {
				if (bIsAkEquipped) {
					InflictDamage(HitResult.GetActor(), 40.f);
				}
				else if (bIsPistolEquipped) {
					InflictDamage(HitResult.GetActor(), 15.f);
				}
				UE_LOG(LogTemp, Warning, TEXT("Hit"));
			}
		}

		if (Role < ROLE_Authority) {
			ServerFireRaycast();
		}
	}
	
}

void AFPSParkourCharacter::ServerFireRaycast_Implementation(){
	FireRaycast();
}

bool AFPSParkourCharacter::ServerFireRaycast_Validate(){
	return true;
}

void AFPSParkourCharacter::PlaySound() {
	// try and play the sound if specified
	if (FireSound != NULL)	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}
}

void AFPSParkourCharacter::MulticastPlaySound_Implementation() {
	PlaySound();
}

void AFPSParkourCharacter::ServerPlaySound_Implementation() {
	MulticastPlaySound();
}

bool AFPSParkourCharacter::ServerPlaySound_Validate() {
	return true;
}

void AFPSParkourCharacter::ServerOnFire_Implementation(){
	OnFire();
}

bool AFPSParkourCharacter::ServerOnFire_Validate(){
	if (bIsAkEquipped) {
		if (RifleCurrentAmmo > 0) {
			return true;
		}
		else {
			return false;
		}
	}
	else if (bIsPistolEquipped) {
		if (PistolCurrentAmmo > 0) {
			return true;
		}
		else {
			return false;
		}
	}
	return true;
}

void AFPSParkourCharacter::SpawnEmitter() {
		if (bIsAkEquipped) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireParticleSystem, FP_Gun->GetSocketLocation("Muzzle"), FP_Gun->GetSocketRotation("Muzzle"), true);
		}
		else if (bIsPistolEquipped) {
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireParticleSystem, FP_Pistol->GetSocketLocation("Muzzle"), FP_Pistol->GetSocketRotation("Muzzle"), true);
		}
}

void AFPSParkourCharacter::MulticastSpawnEmitter_Implementation() {
	SpawnEmitter();
}

void AFPSParkourCharacter::ServerSpawnEmitter_Implementation() {
	MulticastSpawnEmitter();
}

bool AFPSParkourCharacter::ServerSpawnEmitter_Validate() {
	return true;
}

FRotator AFPSParkourCharacter::GetAimOffsets(){
	/*Gets all the aimoffsets and returns them*/
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}

void AFPSParkourCharacter::Sprint(){
	/*Checks if the player is crouching and if not it starts sprinting */
	/*This is used if is the server trying to sprint*/
	if (!bIsCrouching) {
		bIsSprinting = true;
	}
	
	/*Checks if the player is a client*/
	if (Role < ROLE_Authority) {
		ServerSprint();
	}
}

void AFPSParkourCharacter::ServerSprint_Implementation(){
	Sprint();
}

bool AFPSParkourCharacter::ServerSprint_Validate() {
	return true;
}

void AFPSParkourCharacter::ServerDisableSprint_Implementation(){
	DisableSprint();
}

bool AFPSParkourCharacter::ServerDisableSprint_Validate(){
	return true;
}

void AFPSParkourCharacter::DisableSprint() {
	/*This is used if is the server trying to disable sprint*/
	bIsSprinting = false;
	
	if (Role < ROLE_Authority) {
		ServerDisableSprint();
	}
}

void AFPSParkourCharacter::OffFire() {
	/*Clears the timer of the automatic fire*/
	GetWorld()->GetTimerManager().ClearTimer(FireHandle);
	bIsShooting = false;
}

void AFPSParkourCharacter::ServerOnCrouch_Implementation(){
	OnCrouch();
}

bool AFPSParkourCharacter::ServerOnCrouch_Validate(){
	return true;
}

void AFPSParkourCharacter::OnCrouch(){
	/*Checks if the player is sprinting and if not it moves the camera down and starts crouching*/
	/*This is used if is the server trying to Crouch*/
	if (!bIsSprinting) {
		CameraComponent->SetRelativeLocation(
			FVector(128.f,
				25.f,
				20.f));
		bIsCrouching = true;
	}

	/*This is used if the player trying to crouch is a client*/
	if (Role < ROLE_Authority) {
		ServerOnCrouch();
	}
}

void AFPSParkourCharacter::ServerDisableCrouch_Implementation(){
	DisableCrouch();
}

bool AFPSParkourCharacter::ServerDisableCrouch_Validate(){
	return true;
}

void AFPSParkourCharacter::DisableCrouch() {
	/*Moves the camera back up and stop crouching*/
	/*This is used if is the server trying to Disable crouch*/
	CameraComponent->SetRelativeLocation(
		FVector(128.f,
			25.f,
			80.f));
	bIsCrouching = false;

	/*This is used if the player is a client*/
	if (Role < ROLE_Authority) {
		ServerDisableCrouch();
	}
}

void AFPSParkourCharacter::PlayAnimation(UAnimMontage* AnimationToPlay) {
	/*Gets the anim instance of the player and plays the animation*/
	USkeletalMeshComponent* Mesh1P = GetMesh();
	auto AnimInstance = Mesh1P->GetAnimInstance();
	if (AnimInstance && Mesh1P) {
		AnimInstance->Montage_Play(AnimationToPlay, 1.0f);
	}
}

void AFPSParkourCharacter::MulticastPlayAnimation_Implementation(UAnimMontage* AnimationToPlay) {
	PlayAnimation(AnimationToPlay);
}

void AFPSParkourCharacter::ServerPlayAnimation_Implementation(UAnimMontage* AnimationToPlay){
	MulticastPlayAnimation(AnimationToPlay);
}

bool AFPSParkourCharacter::ServerPlayAnimation_Validate(UAnimMontage* AnimationToPlay) {
	return true;
}

void AFPSParkourCharacter::ServerSwitchToPistol_Implementation() {
	SwitchToPistol();
}

bool AFPSParkourCharacter::ServerSwitchToPistol_Validate() {
	return true;
}

void AFPSParkourCharacter::SwitchToPistol() {
	/*Checks if the pistol is not yet equipped and plays the animation and attachs the pistol to the hand of the player and attachs the gun*/
	/*That was equipped to their idle spot*/
	/*This is used if is the server trying to Switch to pistol*/
	if (!bIsPistolEquipped) {
		if (SwitchToPistolAnimMontage) {
			ServerPlayAnimation(SwitchToPistolAnimMontage);
			if (bIsAkEquipped) {
				bIsAkEquipped = false;
				bIsPistolEquipped = true;
				FP_Pistol->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint_Pistol"));
				FP_Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("AKSocket"));
			}
			else {
				bIsKnifeEquipped = false;
				bIsPistolEquipped = true;
				FP_Pistol->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint_Pistol"));
				FP_Knife->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("KnifeSocket"));
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Pistol Montage Not Set"));
		}
	}

	/*This is called if the player is a client*/
	if (Role < ROLE_Authority) {
		ServerSwitchToPistol();
	}
}

void AFPSParkourCharacter::ServerSwitchToRifle_Implementation() {
	SwitchToRifle();
}

bool AFPSParkourCharacter::ServerSwitchToRifle_Validate() {
	return true;
}

void AFPSParkourCharacter::SwitchToRifle() {
	/*Checks if the rifle is not yet equipped and plays the animation and attachs the rifle to the hand of the player and attachs the gun*/
	/*That was equipped to their idle spot*/
	/*This is used if is the server trying to Switch to rifle*/
	if (!bIsAkEquipped) {
		if (SwitchToRifleAnimMontage) {
			ServerPlayAnimation(SwitchToRifleAnimMontage);
			if (bIsPistolEquipped) {
				bIsAkEquipped = true;
				bIsPistolEquipped = false;
				FP_Pistol->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("PistolSocket"));
				FP_Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
			}
			else {
				bIsKnifeEquipped = false;
				bIsAkEquipped = true;
				FP_Knife->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("KnifeSocket"));
				FP_Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Rifle Montage Not Set"));
		}
	}

	/*This is called if the player is a client*/
	if (Role < ROLE_Authority) {
		ServerSwitchToRifle();
	}
}

void AFPSParkourCharacter::ServerSwitchToKnife_Implementation() {
	SwitchToKnife();
}

bool AFPSParkourCharacter::ServerSwitchToKnife_Validate() {
	return true;
}

void AFPSParkourCharacter::SwitchToKnife() {
	/*Checks if the knife is not yet equipped and plays the animation and attachs the knife to the hand of the player and attachs the gun*/
	/*That was equipped to their idle spot*/
	/*This is used if is the server trying to Switch to knife*/
	if (!bIsKnifeEquipped) {
		if (SwitchToKnifeAnimMontage) {
			ServerPlayAnimation(SwitchToKnifeAnimMontage);
			if (bIsPistolEquipped) {
				bIsKnifeEquipped = true;
				bIsPistolEquipped = false;
				FP_Pistol->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("PistolSocket"));
				FP_Knife->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint_Knife"));
			}
			else {
				bIsKnifeEquipped = true;
				bIsAkEquipped = false;
				FP_Knife->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint_Knife"));
				FP_Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("AKSocket"));
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Knife Montage Not Set"));
		}
	}

	/*This is called if the player is a client*/
	if (Role < ROLE_Authority) {
		ServerSwitchToKnife();
	}
}

void AFPSParkourCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Walks Speed: %f"), GetCharacterMovement()->MaxWalkSpeed);
		UE_LOG(LogTemp, Warning, TEXT("Walks Speed: %s"), GetCharacterMovement()->GetIsReplicated() ? TEXT("True") : TEXT("False"));
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AFPSParkourCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFPSParkourCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFPSParkourCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

