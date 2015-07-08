// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "TheArena.h"

//////////////////////////////////////////////////////////////////////////
// AArenaCharacter

AArenaCharacter::AArenaCharacter(const class FObjectInitializer& PCIP)
	: Super(PCIP.SetDefaultSubobjectClass<UArenaCharacterMovement>(ACharacter::CharacterMovementComponentName))
{
	CharacterMovementComponent = Cast<UArenaCharacterMovement>(GetCharacterMovement());
	CharacterAttributes = PCIP.CreateDefaultSubobject<UArenaCharacterAttributes>(this, TEXT("CharacterAttributes"));
	CharacterEquipment = PCIP.CreateDefaultSubobject<UArenaCharacterEquipment>(this, TEXT("CharacterEquipment"));
	CharacterState = PCIP.CreateDefaultSubobject<UArenaCharacterState>(this, TEXT("CharacterState"));
	//Server = ConstructObject<UServer_ArenaCharacter>(UServer_ArenaCharacter::StaticClass());

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	// rotate when the controller rotates
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	//Mesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("PawnMesh"));
	GetMesh()->AttachParent = GetCapsuleComponent();
	GetMesh()->bOnlyOwnerSee = false;
	GetMesh()->bOwnerNoSee = false;
	GetMesh()->bCastDynamicShadow = true;
	GetMesh()->bReceivesDecals = false;
	GetMesh()->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	GetMesh()->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	GetMesh()->bChartDistanceFactor = false;
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->SetNetAddressable();
	GetCharacterMovement()->SetIsReplicated(true);
	GetCharacterMovement()->ForceReplicationUpdate();

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = PCIP.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("CameraBoom"));
	CameraBoom->AttachTo(RootComponent);
	CameraBoom->TargetArmLength = 150.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 50.0f); 
	CameraBoom->bUsePawnControlRotation = true;
	
	// Create a follow camera
	FollowCamera = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FollowCamera"));
	FollowCamera->AttachTo(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//LoadGameInstance = Cast<UArenaSaveGame>(UGameplayStatics::CreateSaveGameObject(UArenaSaveGame::StaticClass()));
}

void AArenaCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Role == ROLE_Authority)
	{
		//initialize components
	}

	// set initial mesh visibility (3rd person view)
	UpdatePawnMeshes();

	// play respawn effects
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (RespawnFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, RespawnFX, GetActorLocation(), GetActorRotation());
		}

		if (RespawnSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, RespawnSound, GetActorLocation());
		}
	}
}

////////////////////////////////////////// Input handlers //////////////////////////////////////////

void AArenaCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// Set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &AArenaCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AArenaCharacter::MoveRight);
	InputComponent->BindAxis("Turn", this, &AArenaCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &AArenaCharacter::LookUpAtRate);

	InputComponent->BindAction("Jump", IE_Pressed, this, &AArenaCharacter::OnStartJump);
	InputComponent->BindAction("Jump", IE_Released, this, &AArenaCharacter::OnStopJump);

	InputComponent->BindAction("Crouch", IE_Pressed, this, &AArenaCharacter::OnCrouch);

	InputComponent->BindAction("Cover", IE_Pressed, this, &AArenaCharacter::OnCover);

	InputComponent->BindAction("Sprint", IE_Pressed, this, &AArenaCharacter::OnStartRunning);
	InputComponent->BindAction("Sprint", IE_Released, this, &AArenaCharacter::OnStopRunning);

	InputComponent->BindAction("Targeting", IE_Pressed, this, &AArenaCharacter::OnStartTargeting);
	InputComponent->BindAction("Targeting", IE_Released, this, &AArenaCharacter::OnStopTargeting);

	InputComponent->BindAction("ReadyWeapon", IE_Pressed, this, &AArenaCharacter::OnEnterCombat);

	InputComponent->BindAction("SwapWeapon", IE_Pressed, this, &AArenaCharacter::OnSwapWeapon);

	InputComponent->BindAction("Reload", IE_Pressed, this, &AArenaCharacter::OnReload);

	InputComponent->BindAction("Melee", IE_Pressed, this, &AArenaCharacter::OnMelee);

	InputComponent->BindAction("Dodge", IE_Pressed, this, &AArenaCharacter::OnDodge);

	InputComponent->BindAction("Fire", IE_Pressed, this, &AArenaCharacter::OnStartFire);
	InputComponent->BindAction("Fire", IE_Released, this, &AArenaCharacter::OnStopFire);
}

void AArenaCharacter::MoveForward(float Value)
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::MoveForward(this, MyPC, Value))
	{
		const bool bLimitRotation = (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling());
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}
void AArenaCharacter::MoveRight(float Value)
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::MoveForward(this, MyPC, Value))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, Value);

		if (Value > 0)
		{
			CharacterMovementComponent->SetDirection(FName(TEXT("Right")));
		}
		if (Value < 0)
		{
			CharacterMovementComponent->SetDirection(FName(TEXT("Left")));
		}
	}
}
void AArenaCharacter::TurnAtRate(float Rate)
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Turn(this, MyPC))
	{
		AddControllerYawInput(Rate * CharacterMovementComponent->GetTurnRate() * GetWorld()->GetDeltaSeconds());
	}
}
void AArenaCharacter::LookUpAtRate(float Rate)
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::LookUp(this, MyPC))
	{
		AddControllerPitchInput(Rate * CharacterMovementComponent->GetLookUpRate() * GetWorld()->GetDeltaSeconds());
	}
}
void AArenaCharacter::OnStartJump()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Jump(this, MyPC))
	{
		CharacterState->SetPlayerState(EPlayerState::Jumping);
		CharacterAttributes->SetCurrentStamina(CharacterAttributes->GetCurrentStamina() - CharacterMovementComponent->GetJumpCost());
		Jump();
	}
}
void AArenaCharacter::OnStopJump()
{
	CharacterState->SetPlayerState(EPlayerState::Default);
}
void AArenaCharacter::OnCrouch()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Crouch(this, MyPC))
	{
		if (CharacterState->GetPlayerState() == EPlayerState::Crouching)
		{
			CharacterState->SetPlayerState(EPlayerState::Default);
			UnCrouch();
		}
		else
		{
			CharacterState->SetPlayerState(EPlayerState::Crouching);
			Crouch();
		}
	}
}
void AArenaCharacter::OnCover()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Cover(this, MyPC))
	{
		if (CharacterState->GetCombatState() == ECombatState::Aggressive)
		{
			CharacterState->SetPlayerState(EPlayerState::Covering);
		}
		if (CharacterState->GetPlayerState() == EPlayerState::Covering)
		{
			CharacterState->SetPlayerState(EPlayerState::Default);
		}
	}
}
void AArenaCharacter::OnStartRunning()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Run(this, MyPC))
	{
		CharacterState->SetPlayerState(EPlayerState::Running);
	}
}
void AArenaCharacter::OnStopRunning()
{
	CharacterState->SetPlayerState(EPlayerState::Default);
}
void AArenaCharacter::OnStartTargeting()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Target(this, MyPC))
	{
		CharacterEquipment->GetCurrentWeapon()->GetWeaponState()->SetTargetingState(ETargetingState::Targeting);
		StartTargeting();
	}
}
void AArenaCharacter::OnStopTargeting()
{
   	if (CharacterEquipment->GetCurrentWeapon())
	{
		GetWorldTimerManager().ClearTimer(this, &AArenaCharacter::LoopTargeting);
		CharacterEquipment->GetCurrentWeapon()->GetWeaponState()->SetTargetingState(ETargetingState::Default);
	}
	/*if (IsLeftEdge() && IsHiCovering())
	{
	PlayAnimMontage(AimHiLeftAnimEnd);
	}
	else if (IsRightEdge() && IsHiCovering())
	{
	PlayAnimMontage(AimHiRightAnimEnd);
	}
	else if (IsLeftEdge() && IsLoCovering())
	{
	PlayAnimMontage(AimLoLeftAnimEnd);
	}
	else if (IsRightEdge() && IsLoCovering())
	{
	PlayAnimMontage(AimLoRightAnimEnd);
	}*/
}
void AArenaCharacter::OnEnterCombat()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Jump(this, MyPC))
	{
		if (CharacterState->GetCombatState() == ECombatState::Passive)
		{
			CharacterState->SetCombatState(ECombatState::Aggressive);
			CharacterEquipment->SetCurrentWeapon();
			EquipWeapon();
		}
		else
		{
			CharacterState->SetCombatState(ECombatState::Passive);
			UnEquipWeapon();
		}
	}
}
void AArenaCharacter::OnSwapWeapon()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Swap(this, MyPC))
	{
		SwapWeapon();
	}
}
void AArenaCharacter::OnReload()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Reload(this, MyPC))
	{
		if (CharacterEquipment->GetCurrentWeapon())
		{
			CharacterEquipment->GetCurrentWeapon()->StartReload();
		}
	}
}
void AArenaCharacter::OnMelee()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Melee(this, MyPC))
	{
		if (CharacterEquipment->GetCurrentWeapon())
		{
			CharacterEquipment->GetCurrentWeapon()->StartMelee();
		}
	}
}
void AArenaCharacter::OnDodge()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{

	}
}
void AArenaCharacter::OnStartFire()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Fire(this, MyPC))
	{
		StartWeaponFire();
	}
}
void AArenaCharacter::OnStopFire()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Fire(this, MyPC))
	{
		StopWeaponFire();
	}
}

////////////////////////////////////////// Character Defaults //////////////////////////////////////////

void AArenaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	{
		CharacterAttributes->Regenerate(DeltaSeconds);
		CharacterMovementComponent->ManageState(DeltaSeconds);
	}

}

void AArenaCharacter::Destroyed()
{
	Super::Destroyed();
	//DestroyInventory();
}

void AArenaCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	UpdatePawnMeshes();
}

void AArenaCharacter::PossessedBy(class AController* InController)
{
	Super::PossessedBy(InController);

}

FRotator AArenaCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}

////////////////////////////////////////// Character Components //////////////////////////////////////////

USkeletalMeshComponent* AArenaCharacter::GetPawnMesh() const
{
	return GetMesh();
}

UArenaCharacterMovement* AArenaCharacter::GetPlayerMovement()
{
	return CharacterMovementComponent;
}

UArenaCharacterState* AArenaCharacter::GetPlayerState() const
{
	return CharacterState;
}

UArenaCharacterAttributes* AArenaCharacter::GetCharacterAttributes()
{
	return CharacterAttributes;
}

UArenaCharacterEquipment* AArenaCharacter::GetCharacterEquipment()
{
	return CharacterEquipment;
}

////////////////////////////////////////// Animation Controls //////////////////////////////////////////

float AArenaCharacter::PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance)
	{
		return UseMesh->AnimScriptInstance->Montage_Play(AnimMontage, InPlayRate);
	}

	return 0.0f;
}

void AArenaCharacter::StopAnimMontage(class UAnimMontage* AnimMontage)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance &&
		UseMesh->AnimScriptInstance->Montage_IsPlaying(AnimMontage))
	{
		UseMesh->AnimScriptInstance->Montage_Stop(AnimMontage->BlendOutTime);
	}
}

void AArenaCharacter::StopAllAnimMontages()
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (UseMesh && UseMesh->AnimScriptInstance)
	{
		UseMesh->AnimScriptInstance->Montage_Stop(0.0f);
	}
}

////////////////////////////////////////// Action Functions //////////////////////////////////////////

void AArenaCharacter::OnStartVault(bool bFromReplication)//recall3
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (!bFromReplication && Role < ROLE_Authority)
	{
		ServerStartVault();
	}

	if (bFromReplication || true)
	{
		CharacterState->SetPlayerState(EPlayerState::Vaulting);
		GetCapsuleComponent()->SetCapsuleSize(0, 0);
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		float AnimDuration = PlayAnimMontage(CharacterMovementComponent->GetVaultAnimation());
		if (AnimDuration <= 0.0f)
		{
			AnimDuration = 0.3f;
		}

		GetWorldTimerManager().SetTimer(this, &AArenaCharacter::OnStopVault, AnimDuration, false);

		if (MyPC)
		{
			//PlayWeaponSound(MeleeSound);
		}
	}
}
void AArenaCharacter::OnStopVault()
{
	CharacterState->SetPlayerState(EPlayerState::Default);
	GetCapsuleComponent()->SetCapsuleSize(42.f, 96.0f);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	StopAnimMontage(CharacterMovementComponent->GetVaultAnimation());
}

void AArenaCharacter::EquipWeapon()
{
	if (Role == ROLE_Authority)
	{
		CharacterEquipment->SetCurrentWeapon();
		FinishEquipWeapon(CharacterEquipment->GetCurrentWeapon());
	}
	else
	{
		ServerEquipWeapon();
	}
}
void AArenaCharacter::FinishEquipWeapon(class AArenaWeapon* Weapon)
{
	if (Weapon->IsPrimary() == true)
	{
		Weapon->SetOwningPawn(this);
		Weapon->Equip();
	}
	else
	{
		Weapon->SetOwningPawn(this);
		Weapon->Equip();
	}
}

float AArenaCharacter::UnEquipWeapon()
{
	float Duration;
	if (Role == ROLE_Authority)
	{
		Duration = FinishUnEquipWeapon(CharacterEquipment->GetCurrentWeapon());
	}
	else
	{
		ServerUnEquipWeapon();
	}
	return Duration;
}
float AArenaCharacter::FinishUnEquipWeapon(class AArenaWeapon* Weapon)
{
	float Duration;
	if (Weapon->IsPrimary() == true)
	{
		Weapon->SetOwningPawn(this);
		Duration = Weapon->UnEquip();
	}
	else
	{
		Weapon->SetOwningPawn(this);
		Duration = Weapon->UnEquip();
	}
	return Duration;
}

void AArenaCharacter::StartTargeting(bool bFromReplication)//recall3
{
	if (!bFromReplication && Role < ROLE_Authority)
	{
		ServerStartTargeting();
	}

	if (bFromReplication || true)
	{
		float Duration = 0.0f;

		/*if (IsLeftEdge() && IsHiCovering())
		{
		Duration = PlayAnimMontage(AimHiLeftAnimStart);
		}
		else if (IsRightEdge() && IsHiCovering())
		{
		Duration = PlayAnimMontage(AimHiRightAnimStart);
		}
		else if (IsLeftEdge() && IsLoCovering())
		{
		Duration = PlayAnimMontage(AimLoLeftAnimStart);
		}
		else if (IsRightEdge() && IsLoCovering())
		{
		Duration = PlayAnimMontage(AimLoRightAnimStart);
		}*/

		GetWorldTimerManager().SetTimer(this, &AArenaCharacter::LoopTargeting, 0.6f, false);
	}
}
void AArenaCharacter::SetTargeting(bool bNewTargeting)
{
	if (TargetingSound)
	{
		UGameplayStatics::PlaySoundAttached(TargetingSound, GetRootComponent());
	}

	if (Role < ROLE_Authority)
	{
		ServerSetTargeting(bNewTargeting);
	}
}
void AArenaCharacter::LoopTargeting()
{
	/*if (IsLeftEdge() && IsHiCovering())
	{
	PlayAnimMontage(AimHiLeftAnimLoop);
	}
	else if (IsRightEdge() && IsHiCovering())
	{
	PlayAnimMontage(AimHiRightAnimLoop);
	}
	else if (IsLeftEdge() && IsLoCovering())
	{
	PlayAnimMontage(AimLoLeftAnimLoop);
	}
	else if (IsRightEdge() && IsLoCovering())
	{
	PlayAnimMontage(AimLoRightAnimLoop);
	}*/
}

void AArenaCharacter::SwapWeapon()
{
	float Duration = UnEquipWeapon();
	GetWorldTimerManager().SetTimer(TimerHandle_SwapWeapon, this, &AArenaCharacter::EquipWeapon, Duration * 0.75f, false);
}

void AArenaCharacter::StartWeaponFire()
{
	CharacterEquipment->GetCurrentWeapon()->StartAttack();
}
void AArenaCharacter::StopWeaponFire()
{
	CharacterEquipment->GetCurrentWeapon()->StopAttack();
}

////////////////////////////////////////// Damage & Death //////////////////////////////////////////

float AArenaCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (CharacterAttributes->GetCurrentHealth() <= 0.f)
	{
		return 0.f;
	}

	// Modify based on game rules.
	//AArenaGameMode* const Game = GetWorld()->GetAuthGameMode<AArenaGameMode>();
	//Damage = Game ? Game->ModifyDamage(Damage, this, DamageEvent, EventInstigator, DamageCauser) : 0.f;

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		CharacterAttributes->SetCurrentHealth(CharacterAttributes->GetCurrentHealth() - ActualDamage);
		if (CharacterAttributes->GetCurrentHealth() <= 0)
		{
			Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
		}
		else
		{
			PlayHit(ActualDamage, DamageEvent, EventInstigator ? EventInstigator->GetPawn() : NULL, DamageCauser);
		}

		MakeNoise(1.0f, EventInstigator ? EventInstigator->GetPawn() : this);
	}

	return ActualDamage;
}

void AArenaCharacter::PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	if (Role == ROLE_Authority)
	{
		ReplicateHit(DamageTaken, DamageEvent, PawnInstigator, DamageCauser, false);

		// play the force feedback effect on the client player controller
		APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC && DamageEvent.DamageTypeClass)
		{
			UArenaDamageType *DamageType = Cast<UArenaDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
			if (DamageType && DamageType->HitForceFeedback)
			{
				PC->ClientPlayForceFeedback(DamageType->HitForceFeedback, false, "Damage");
			}
		}
	}

	if (DamageTaken > 0.f)
	{
		ApplyDamageMomentum(DamageTaken, DamageEvent, PawnInstigator, DamageCauser);
	}

	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	//AArenaHUD* MyHUD = MyPC ? Cast<AArenaHUD>(MyPC->GetHUD()) : NULL;
	//if (MyHUD)
	//{
	//MyHUD->NotifyHit(DamageTaken, DamageEvent, PawnInstigator);
	//}

	if (PawnInstigator && PawnInstigator != this && PawnInstigator->IsLocallyControlled())
	{
		AArenaPlayerController* InstigatorPC = Cast<AArenaPlayerController>(PawnInstigator->Controller);
		//AArenaHUD* InstigatorHUD = InstigatorPC ? Cast<AArenaHUD>(InstigatorPC->GetHUD()) : NULL;
		//if (InstigatorHUD)
		//{
		//InstigatorHUD->NotifyEnemyHit();
		//}
	}
}

bool AArenaCharacter::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{
	if (ArenaCharacterCan::Die(this))
	{
		return false;
	}

	CharacterAttributes->SetCurrentHealth(FMath::Min(0.0f, 1000.0f));

	// if this is an environmental death then refer to the previous killer so that they receive credit (knocked into lava pits, etc)
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	AController* const KilledPlayer = (Controller != NULL) ? Controller : Cast<AController>(GetOwner());
	//GetWorld()->GetAuthGameMode<AArenaGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);

	NetUpdateFrequency = GetDefault<AArenaCharacter>()->NetUpdateFrequency;
	GetCharacterMovement()->ForceReplicationUpdate();

	OnDeath(KillingDamage, DamageEvent, Killer ? Killer->GetPawn() : NULL, DamageCauser);
	return true;
}

void AArenaCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	if (CharacterAttributes->GetCurrentHealth() <= 0)
	{
		return;
	}

	bReplicateMovement = false;
	bTearOff = true;

	if (Role == ROLE_Authority)
	{
		ReplicateHit(KillingDamage, DamageEvent, PawnInstigator, DamageCauser, true);

		// play the force feedback effect on the client player controller
		APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC && DamageEvent.DamageTypeClass)
		{
			UArenaDamageType *DamageType = Cast<UArenaDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
			if (DamageType && DamageType->KilledForceFeedback)
			{
				PC->ClientPlayForceFeedback(DamageType->KilledForceFeedback, false, "Damage");
			}
		}
	}

	// cannot use IsLocallyControlled here, because even local client's controller may be NULL here
	if (GetNetMode() != NM_DedicatedServer && DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}

	// remove all weapons
	//DestroyInventory();

	// switch back to 3rd person view
	UpdatePawnMeshes();

	DetachFromControllerPendingDestroy();
	StopAllAnimMontages();

	/*if (LowHealthWarningPlayer && LowHealthWarningPlayer->IsPlaying())
	{
	LowHealthWarningPlayer->Stop();
	}*/

	if (CharacterMovementComponent->GetRunLoopAC())
	{
		CharacterMovementComponent->GetRunLoopAC()->Stop();
	}

	// disable collisions on capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	if (GetMesh())
	{
		static FName CollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetCollisionProfileName(CollisionProfileName);
	}
	SetActorEnableCollision(true);

	// Death anim
	//float DeathAnimDuration = PlayAnimMontage(DeathAnim); maybe rage doll is better

	// Ragdoll
	//if (DeathAnimDuration > 0.f)
	//{
	//GetWorldTimerManager().SetTimer(this, &AArenaCharacter::SetRagdollPhysics, FMath::Min(0.1f, DeathAnimDuration), false);
	//}
	//else
	//{
	SetRagdollPhysics();
	//}
}

void AArenaCharacter::FellOutOfWorld(const class UDamageType& dmgType)
{
	Die(CharacterAttributes->GetCurrentHealth(), FDamageEvent(dmgType.GetClass()), NULL, NULL);
}

void AArenaCharacter::Suicide()
{
	KilledBy(this);
}void AArenaCharacter::SetRagdollPhysics()
{
	bool bInRagdoll = false;

	if (IsPendingKill())
	{
		bInRagdoll = false;
	}
	else if (!GetMesh() || !GetMesh()->GetPhysicsAsset())
	{
		bInRagdoll = false;
	}
	else
	{
		// initialize physics/etc
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;

		bInRagdoll = true;
	}

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);

	if (!bInRagdoll)
	{
		// hide and set short lifespan
		TurnOff();
		SetActorHiddenInGame(true);
		SetLifeSpan(1.0f);
	}
	else
	{
		SetLifeSpan(10.0f);
	}
}

void AArenaCharacter::KilledBy(APawn* EventInstigator)
{
	if (Role == ROLE_Authority && !bIsDying)
	{
		AController* Killer = NULL;
		if (EventInstigator != NULL)
		{
			Killer = EventInstigator->Controller;
			LastHitBy = NULL;
		}

		Die(CharacterAttributes->GetCurrentHealth(), FDamageEvent(UDamageType::StaticClass()), Killer, NULL);
	}
}

////////////////////////////////////////// Pawn Handeling //////////////////////////////////////////

void AArenaCharacter::UpdatePawnMeshes()
{
	//Mesh->MeshComponentUpdateFlag = bFirstPerson ? EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered : EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;
	//Mesh->SetOwnerNoSee(bFirstPerson);
}

void AArenaCharacter::TornOff()
{
	SetLifeSpan(25.f);
}

/*void AArenaCharacter::AttackTrace()
{
	//Overlapping actors for each box spawned will be stored here
	TArray<struct FOverlapResult> OutOverlaps;
	//The initial rotation of our box is the same as our character rotation
	FQuat Rotation = Instigator->GetTransform().GetRotation();
	FVector Start = Instigator->GetTransform().GetLocation() + Rotation.Rotator().Vector() * 100.0f;

	FCollisionShape CollisionHitShape;
	FCollisionQueryParams CollisionParams;
	//We do not want to store the instigator character in the array, so ignore it's collision
	CollisionParams.AddIgnoredActor(Instigator);

	//Set the channels that will respond to the collision
	FCollisionObjectQueryParams CollisionObjectTypes;
	CollisionObjectTypes.AddObjectTypesToQuery(ECollisionChannel::ECC_PhysicsBody);
	CollisionObjectTypes.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
	CollisionObjectTypes.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);

	//Create the box and get the overlapping actors
	CollisionHitShape = FCollisionShape::MakeBox(FVector(60.0f, 60.0f, 0.5f));
	GetWorld()->OverlapMulti(OutOverlaps, Start, Rotation, CollisionHitShape, CollisionParams, CollisionObjectTypes);

	//Process all hit actors
	for (int i = 0; i < OutOverlaps.Num(); ++i)
	{
		//We process each actor only once per Attack execution
		if (OutOverlaps[i].GetActor() && !HitActors.Contains(OutOverlaps[i].GetActor()))
		{
			//Process the actor to deal damage
			CurrentWeapon->Melee(OutOverlaps[i].GetActor(), HitActors);
			ServerMeleeAttack(CurrentWeapon, OutOverlaps[i].GetActor(), HitActors);
		}
	}
}*/

////////////////////////////////////////// Audio Controls //////////////////////////////////////////

void AArenaCharacter::UpdateRunSounds(bool bNewRunning)
{
	if (bNewRunning)
	{
		if (!CharacterMovementComponent->GetRunLoopAC() && CharacterMovementComponent->GetRunLoopSound())
		{
			CharacterMovementComponent->SetRunLoopAC(UGameplayStatics::PlaySoundAttached(CharacterMovementComponent->GetRunLoopSound(), GetRootComponent()));
			if (CharacterMovementComponent->GetRunLoopAC())
			{
				CharacterMovementComponent->GetRunLoopAC()->bAutoDestroy = false;
			}
		}
		else if (CharacterMovementComponent->GetRunLoopAC())
		{
			CharacterMovementComponent->GetRunLoopAC()->Play();
		}
	}
	else
	{
		if (CharacterMovementComponent->GetRunLoopAC())
		{
			CharacterMovementComponent->GetRunLoopAC()->Stop();
		}
		if (CharacterMovementComponent->GetRunStopSound())
		{
			UGameplayStatics::PlaySoundAttached(CharacterMovementComponent->GetRunStopSound(), GetRootComponent());
		}
	}
}

/////////////////////////////////////////// Replication ///////////////////////////////////////////

void AArenaCharacter::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	// Only replicate this property for a short duration after it changes so join in progress players don't get spammed with fx when joining late
	DOREPLIFETIME_ACTIVE_OVERRIDE(AArenaCharacter, LastTakeHitInfo, GetWorld() && GetWorld()->GetTimeSeconds() < LastTakeHitTimeTimeout);
}

void AArenaCharacter::ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser, bool bKilled)
{
	const float TimeoutTime = GetWorld()->GetTimeSeconds() + 0.5f;

	FDamageEvent const& LastDamageEvent = LastTakeHitInfo.GetDamageEvent();
	if ((PawnInstigator == LastTakeHitInfo.PawnInstigator.Get()) && (LastDamageEvent.DamageTypeClass == LastTakeHitInfo.DamageTypeClass) && (LastTakeHitTimeTimeout == TimeoutTime))
	{
		// same frame damage
		if (bKilled && LastTakeHitInfo.bKilled)
		{
			// Redundant death take hit, just ignore it
			return;
		}

		// otherwise, accumulate damage done this frame
		Damage += LastTakeHitInfo.ActualDamage;
	}

	LastTakeHitInfo.ActualDamage = Damage;
	LastTakeHitInfo.PawnInstigator = Cast<AArenaCharacter>(PawnInstigator);
	LastTakeHitInfo.DamageCauser = DamageCauser;
	LastTakeHitInfo.SetDamageEvent(DamageEvent);
	LastTakeHitInfo.bKilled = bKilled;
	LastTakeHitInfo.EnsureReplication();

	LastTakeHitTimeTimeout = TimeoutTime;
}

void AArenaCharacter::OnRep_LastTakeHitInfo()
{
	if (LastTakeHitInfo.bKilled)
	{
		OnDeath(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get());
	}
	else
	{
		PlayHit(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get());
	}
}

void AArenaCharacter::OnRep_Vault()
{
	/*if (bWantsToVault)
	{
	OnStartVault(true);
	}
	else
	{
	OnStopVault();
	}*/
}

void AArenaCharacter::OnRep_Aim()
{
	//if (bWantsToAim)
	//{
	//	StartTargeting();
	//}
	//else
	//{
	//	OnStopTargeting();
	//}
}

void AArenaCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AArenaCharacter, Inventory, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AArenaCharacter, LastTakeHitInfo, COND_Custom);
}

////////////////////////////////////////////// Server //////////////////////////////////////////////

bool AArenaCharacter::ServerEquipWeapon_Validate()
{
	return true;
}

void AArenaCharacter::ServerEquipWeapon_Implementation()
{
	EquipWeapon();
}

bool AArenaCharacter::ServerUnEquipWeapon_Validate()
{
	return true;
}

void AArenaCharacter::ServerUnEquipWeapon_Implementation()
{
	UnEquipWeapon();
}

bool AArenaCharacter::ServerSetTargeting_Validate(bool bNewTargeting)
{
	return true;
}

void AArenaCharacter::ServerSetTargeting_Implementation(bool bNewTargeting)
{
	SetTargeting(bNewTargeting);
}

bool AArenaCharacter::ServerSetRunning_Validate(bool bNewRunning, bool bToggle)
{
	return true;
}

void AArenaCharacter::ServerSetRunning_Implementation(bool bNewRunning, bool bToggle)
{
	//SetRunning(bNewRunning, bToggle);
}

bool AArenaCharacter::ServerSetCrouched_Validate(bool bNewCrouched, bool bToggle)
{
	return true;
}

void AArenaCharacter::ServerSetCrouched_Implementation(bool bNewCrouched, bool bToggle)
{
	//SetCrouched(bNewCrouched, bToggle);
}

bool AArenaCharacter::ServerJump_Validate(class AArenaCharacter* client)
{
	return true;
}

void AArenaCharacter::ServerJump_Implementation(class AArenaCharacter* client)
{
	//client->PlayerConfig.Stamina -= JumpCost;
}

bool AArenaCharacter::ServerStartVault_Validate()
{
	return true;
}

void AArenaCharacter::ServerStartVault_Implementation()
{
	OnStartVault();
}

bool AArenaCharacter::ServerStopVault_Validate()
{
	return true;
}

void AArenaCharacter::ServerStopVault_Implementation()
{
	OnStopVault();
}

bool AArenaCharacter::ServerStartTargeting_Validate()
{
	return true;
}

void AArenaCharacter::ServerStartTargeting_Implementation()
{
	StartTargeting();
}

bool AArenaCharacter::ServerStopTargeting_Validate()
{
	return true;
}

void AArenaCharacter::ServerStopTargeting_Implementation()
{
	OnStopTargeting();
}

