// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "TheArena.h"

//////////////////////////////////////////////////////////////////////////
// AArenaCharacter

AArenaCharacter::AArenaCharacter(const class FObjectInitializer& PCIP)
	: Super(PCIP.SetDefaultSubobjectClass<UArenaCharacterMovement>(ACharacter::CharacterMovementComponentName))
{
	bWantsToRun = false;
	bWantsToFire = false;
	bWantsToThrow = false;
	bPressedDodgeRight = false;
	bPressedDodgeLeft = false;
	bPressedDodgeForward = false;
	bPressedDodgeBack = false;
	BaseMovementSpeed = 400.0f;
	TargetingMovementSpeed = 280.0f;
	RunningMovementSpeed = 520.0f;
	CrouchedMovementSpeed = 340.0f;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_MAX);

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
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
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

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
}

void AArenaCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Role == ROLE_Authority)
	{
		IdleTime = 0.0f;
		bInCombat = false;
		PlayerConfig.Health = GetMaxHealth();
		PlayerConfig.Stamina = GetMaxStamina();
		PlayerConfig.Energy = GetMaxEnergy();
		SpawnDefaultInventory();
	}

	// set initial mesh visibility (3rd person view)
	UpdatePawnMeshes();

	// create material instance for setting team colors (3rd person view)
	for (int32 iMat = 0; iMat < GetMesh()->GetNumMaterials(); iMat++)
	{
		MeshMIDs.Add(GetMesh()->CreateAndSetMaterialInstanceDynamic(iMat));
	}

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

void AArenaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);

	if (this->PlayerConfig.Energy < this->GetMaxEnergy())
	{
		this->PlayerConfig.Energy += PlayerConfig.EnergyRegen * DeltaSeconds;
		if (PlayerConfig.Energy > this->GetMaxEnergy())
		{
			PlayerConfig.Energy = this->GetMaxEnergy();
		}
	}

	if ((this->PlayerConfig.Stamina < this->GetMaxStamina()) && (!IsRunning()))
	{
		this->PlayerConfig.Stamina += PlayerConfig.StaminaRegen * DeltaSeconds;
		if (PlayerConfig.Stamina > this->GetMaxStamina())
		{
			PlayerConfig.Stamina = this->GetMaxStamina();
		}
	}

	if (this->PlayerConfig.Health < this->GetMaxHealth())
	{
		this->PlayerConfig.Health += PlayerConfig.HealthRegen * DeltaSeconds;
		if (PlayerConfig.Health > this->GetMaxHealth())
		{
			PlayerConfig.Health = this->GetMaxHealth();
		}
	}

	if (this->PlayerConfig.Stamina <= 0)
	{
		OnStopRunning();
	}

	if (IsRunning())
	{
		this->PlayerConfig.Stamina -= SprintCost * DeltaSeconds;
	}

	else if (!IsRunning())
	{
		SetRunning(false, false);
	}

	if (LowHealthSound && GEngine->UseSound())
	{
		if ((this->PlayerConfig.Health > 0 && this->PlayerConfig.Health < this->GetMaxHealth() * PlayerConfig.LowHealthPercentage) && (!LowHealthWarningPlayer || !LowHealthWarningPlayer->IsPlaying()))
		{
			LowHealthWarningPlayer = UGameplayStatics::PlaySoundAttached(LowHealthSound, GetRootComponent(),
				NAME_None, FVector(ForceInit), EAttachLocation::KeepRelativeOffset, true);
			LowHealthWarningPlayer->SetVolumeMultiplier(0.0f);
		}
		else if ((this->PlayerConfig.Health > this->GetMaxHealth() * PlayerConfig.LowHealthPercentage || this->PlayerConfig.Health < 0) && LowHealthWarningPlayer && LowHealthWarningPlayer->IsPlaying())
		{
			LowHealthWarningPlayer->Stop();
		}
		if (LowHealthWarningPlayer && LowHealthWarningPlayer->IsPlaying())
		{
			const float MinVolume = 0.3f;
			const float VolumeMultiplier = (1.0f - (this->PlayerConfig.Health / (this->GetMaxHealth() * PlayerConfig.LowHealthPercentage)));
			LowHealthWarningPlayer->SetVolumeMultiplier(MinVolume + (1.0f - MinVolume) * VolumeMultiplier);
		}
	}
	this->IdleTime += DeltaSeconds;
	ServerIdleTimer(IdleTime, this);
	CameraUpdate();
}

void AArenaCharacter::Destroyed()
{
	Super::Destroyed();
	DestroyInventory();
}

void AArenaCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	// switch mesh to 1st person view
	UpdatePawnMeshes();

	// reattach weapon if needed
	//SetCurrentWeapon(CurrentWeapon);

	// set team colors for 1st person view
	UMaterialInstanceDynamic* Mesh3PMID = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
	UpdateTeamColors(Mesh3PMID);
}

void AArenaCharacter::PossessedBy(class AController* InController)
{
	Super::PossessedBy(InController);

	// [server] as soon as PlayerState is assigned, set team colors of this pawn for local player
	UpdateTeamColorsAllMIDs();
}

void AArenaCharacter::CameraUpdate()
{
	if (IsRunning())
	{
		CameraBoom->TargetArmLength = 175.0f;
		CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 0.0f);
	}
	else if (IsTargeting())
	{
		CameraBoom->TargetArmLength = 50.0f;
		CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 50.0f);
	}
	else if (GetCombat() == false)
	{
		CameraBoom->TargetArmLength = 250.0f;
		CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 0.0f);
	}
	else
	{
		CameraBoom->TargetArmLength = 150.0f;
		CameraBoom->SocketOffset = FVector(0.0f, 50.0f, 50.0f);
	}
}

FRotator AArenaCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}

bool AArenaCharacter::IsEnemyFor(AController* TestPC) const
{
	if (TestPC == Controller || TestPC == NULL)
	{
		return false;
	}

	//AShooterPlayerState* TestPlayerState = Cast<AShooterPlayerState>(TestPC->PlayerState);
	//AShooterPlayerState* MyPlayerState = Cast<AShooterPlayerState>(PlayerState);

	bool bIsEnemy = true;
	if (GetWorld()->GameState && GetWorld()->GameState->GameModeClass)
	{
		/*const AShooterGameMode* DefGame = GetWorld()->GameState->GameModeClass->GetDefaultObject<AShooterGameMode>();
		if (DefGame && MyPlayerState && TestPlayerState)
		{
			bIsEnemy = DefGame->CanDealDamage(TestPlayerState, MyPlayerState);
		}*/
	}

	return bIsEnemy;
}

//////////////////////////////////////////////////////////////////////////
// Inventory

void AArenaCharacter::AddWeapon(AArenaRangedWeapon* Weapon)
{
	if (Weapon && Role == ROLE_Authority && Inventory.Num() < 2)
	{
		Weapon->OnEnterInventory(this);
		Inventory.AddUnique(Weapon);
	}
}

void AArenaCharacter::RemoveWeapon(AArenaRangedWeapon* Weapon)
{
	if (Weapon && Role == ROLE_Authority)
	{
		Weapon->OnLeaveInventory();
		Inventory.RemoveSingle(Weapon);
	}
}

AArenaRangedWeapon* AArenaCharacter::FindWeapon(TSubclassOf<AArenaRangedWeapon> WeaponClass)
{
	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] && Inventory[i]->IsA(WeaponClass))
		{
			return Inventory[i];
		}
	}

	return NULL;
}

void AArenaCharacter::EquipWeapon(AArenaRangedWeapon* ToEquip, AArenaRangedWeapon* ToUnEquip, bool IsEnteringCombat)
{
	if (ToEquip)
	{
		if (Role == ROLE_Authority)
		{
			if (IsEnteringCombat == true)
			{
				SetCurrentWeapon(ToEquip, ToUnEquip);
				FinishEquipWeapon();
			}
			else
			{
				SetCurrentWeapon(ToEquip, ToUnEquip);
				StartEquipWeapon();
				GetWorldTimerManager().SetTimer(this, &AArenaCharacter::FinishEquipWeapon, 1.5f, false);
			}
		}	
		else
		{
			ServerEquipWeapon(ToEquip, ToUnEquip, IsEnteringCombat);
		}
		IsEnteringCombat = false;
	}
}

void AArenaCharacter::InitializeWeapons(AArenaRangedWeapon* mainWeapon, AArenaRangedWeapon* offWeapon)
{
	if (Role == ROLE_Authority)
	{
		mainWeapon->OnHolster(true);
		offWeapon->OnHolster(false);
	}
	else
	{
		ServerInitializeWeapons(mainWeapon, offWeapon);
	}
}

//////////////////////////////////////////////////////////////////////////
// Weapon usage

void AArenaCharacter::StartWeaponFire()
{
	if (!bWantsToFire && bInCombat)
	{
		bWantsToFire = true;
		if (CurrentWeapon)
		{
			CurrentWeapon->StartFire();
		}
	}
}

void AArenaCharacter::StopWeaponFire()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFire();
		}
	}
}

bool AArenaCharacter::CanFire() const
{
	return IsAlive();
}

bool AArenaCharacter::CanReload() const
{
	return true;
}

bool AArenaCharacter::CanMelee() const
{
	return true;
}

void AArenaCharacter::SetTargeting(bool bNewTargeting)
{
	bIsTargeting = bNewTargeting;

	if (TargetingSound)
	{
		UGameplayStatics::PlaySoundAttached(TargetingSound, GetRootComponent());
	}

	if (Role < ROLE_Authority)
	{
		ServerSetTargeting(bNewTargeting);
	}
}

void AArenaCharacter::SetCombat(bool bNewCombatState)
{
	this->bInCombat = bNewCombatState;

	if (Role < ROLE_Authority)
	{
		ServerSetCombat(bNewCombatState);
	}
	
	UpdateCombatState();
	
}

void AArenaCharacter::UpdateCombatState()
{
	if (bInCombat == true)
	{
		bUseControllerRotationYaw = true;
		OnSwapWeapon();
	}
	else
	{
		bUseControllerRotationYaw = false;
		if (CurrentWeapon->GetIsPrimaryWeapon() == true)
		{
			PrimaryWeapon->OnUnEquip(true);
		}
		else
		{
			SecondaryWeapon->OnUnEquip(false);
		}
		CurrentWeapon = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// Movement

void AArenaCharacter::SetRunning(bool bNewRunning, bool bToggle)
{
	bWantsToRun = bNewRunning;

	if (Role < ROLE_Authority)
	{
		ServerSetRunning(bNewRunning, bToggle);
	}

	UpdateRunSounds(bNewRunning);
}

void AArenaCharacter::SetCrouched(bool bNewCrouched, bool bToggle)
{
	bWantsToCrouch = bNewCrouched;
	if (Role < ROLE_Authority)
	{
		ServerSetCrouched(bNewCrouched, bToggle);
	}
}

//////////////////////////////////////////////////////////////////////////
// Animations

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

//////////////////////////////////////////////////////////////////////////
// Input handlers

void AArenaCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// Set up gameplay key bindings
	check(InputComponent);
	InputComponent->BindAxis("MoveForward", this, &AArenaCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AArenaCharacter::MoveRight);
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &AArenaCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AArenaCharacter::LookUpAtRate);

	InputComponent->BindAction("Jump", IE_Pressed, this, &AArenaCharacter::OnStartJump);
	InputComponent->BindAction("Jump", IE_Released, this, &AArenaCharacter::OnStopJump);

	InputComponent->BindAction("Crouch", IE_Pressed, this, &AArenaCharacter::OnStartCrouching);
	InputComponent->BindAction("Crouch", IE_Released, this, &AArenaCharacter::OnStopCrouching);

	InputComponent->BindAction("Sprint", IE_Pressed, this, &AArenaCharacter::OnStartRunning);
	InputComponent->BindAction("Sprint", IE_Released, this, &AArenaCharacter::OnStopRunning);

	InputComponent->BindAction("Targeting", IE_Pressed, this, &AArenaCharacter::OnStartTargeting);
	InputComponent->BindAction("Targeting", IE_Released, this, &AArenaCharacter::OnStopTargeting);

	InputComponent->BindAction("Cover", IE_Pressed, this, &AArenaCharacter::OnEnterCover);
	InputComponent->BindAction("Cover", IE_Released, this, &AArenaCharacter::OnExitCover);

	InputComponent->BindAction("SwapWeapon", IE_Pressed, this, &AArenaCharacter::OnSwapWeapon);

	InputComponent->BindAction("Reload", IE_Pressed, this, &AArenaCharacter::OnReload);

	InputComponent->BindAction("Melee", IE_Pressed, this, &AArenaCharacter::OnMelee);

	InputComponent->BindAction("Dodge", IE_Pressed, this, &AArenaCharacter::OnDodge);

	InputComponent->BindAction("Fire", IE_Pressed, this, &AArenaCharacter::OnStartFire);
	InputComponent->BindAction("Fire", IE_Released, this, &AArenaCharacter::OnStopFire);

	InputComponent->BindAction("Throw", IE_Pressed, this, &AArenaCharacter::OnThrow);
}

void AArenaCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		IdleTime = 0.0f;
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		MovementForwardAxis = Value;
		AddMovementInput(Direction, Value);
	}
}

void AArenaCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && (!bWantsToRun))
	{
		IdleTime = 0.0f;
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		MovementStrafeAxis = Value;
		AddMovementInput(Direction, Value);
	}
}

void AArenaCharacter::TurnAtRate(float Rate)
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		// calculate delta for this frame from the rate information
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}

void AArenaCharacter::LookUpAtRate(float Rate)
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		// calculate delta for this frame from the rate information
		AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
}

void AArenaCharacter::OnStartFire()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		IdleTime = 0.0f;
		if (IsRunning())
		{
			SetRunning(false, false);
		}
		StartWeaponFire();
	}
}

void AArenaCharacter::OnStopFire()
{
	StopWeaponFire();
}

void AArenaCharacter::StartThrow(bool bFromReplication)
{
	if (bFromReplication || Role < ROLE_Authority)
	{
		ServerStartThrow();
	}

	if (bFromReplication || bWantsToThrow != true)
	{
		bWantsToThrow = true;
		this->PlayerConfig.Energy -= 500;

		float AnimDuration = PlayWeaponAnimation(ThrowAnimation);
		if (AnimDuration <= 0.0f)
		{
			AnimDuration = 0.7f;
		}

		GetWorldTimerManager().SetTimer(this, &AArenaCharacter::StopThrow, AnimDuration, false);
		if (Role == ROLE_Authority)
		{
			GetWorldTimerManager().SetTimer(this, &AArenaCharacter::Throw, FMath::Max(0.1f, AnimDuration - 1.5f), false);
		}

		if (this && this->IsLocallyControlled())
		{
			//PlayWeaponSound(ThrowSound);
		}
	}
}

void AArenaCharacter::StopThrow()
{
	bWantsToThrow = false;
}

void AArenaCharacter::Throw()
{
	FVector FinalAim = FVector::ZeroVector;
	FVector ShootDir = Instigator->GetBaseAimRotation().Vector();

	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	FVector Origin = UseMesh->GetSocketLocation(OffHandAttachPoint);

	FTransform SpawnTM(ShootDir.Rotation(), Origin);
	AArenaFragGrenade* grenade = Cast<AArenaFragGrenade>(UGameplayStatics::BeginSpawningActorFromClass(this, GrenadeClass, SpawnTM));
	if (grenade)
	{
		//grenade->SetPawnOwner(this);
		grenade->Instigator = Instigator;
		grenade->SetOwner(this);
		grenade->InitVelocity(ShootDir);

		UGameplayStatics::FinishSpawningActor(grenade, SpawnTM);
	}
}

float AArenaCharacter::PlayWeaponAnimation(UAnimMontage* Animation)
{
	float Duration = 0.0f;
	if (this)
	{
		UAnimMontage* UseAnim = Animation;
		if (UseAnim)
		{
			Duration = this->PlayAnimMontage(UseAnim);
		}
	}

	return Duration;
}

void AArenaCharacter::OnStartTargeting()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed() && bInCombat == true)
	{
		IdleTime = 0.0f;
		if (IsRunning())
		{
			SetRunning(false, false);
		}
		GetCharacterMovement()->MaxWalkSpeed = TargetingMovementSpeed;
		SetTargeting(true);
	}
}

void AArenaCharacter::OnStopTargeting()
{
	if (bInCombat == true)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		SetTargeting(false);
	}
}

void AArenaCharacter::OnEnterCover()
{

}

void AArenaCharacter::OnExitCover()
{

}

void AArenaCharacter::OnSwapWeapon()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed() && bInCombat == true)
	{
		if (Inventory.Num() >= 2 && (CurrentWeapon == NULL || CurrentWeapon->GetCurrentState() != EWeaponState::Equipping))
		{
			if (CurrentWeapon == NULL)
			{
				EquipWeapon(PrimaryWeapon, NULL, true);
			}
			else if (CurrentWeapon == PrimaryWeapon)
			{
				EquipWeapon(SecondaryWeapon, PrimaryWeapon, false);
			}
			else
			{
				EquipWeapon(PrimaryWeapon, SecondaryWeapon, false);
			}
			/*const int32 CurrentWeaponIdx = Inventory.IndexOfByKey(CurrentWeapon);
			AArenaRangedWeapon* NextWeapon = Inventory[(CurrentWeaponIdx + 1) % Inventory.Num()];
			EquipWeapon(NextWeapon, false);*/
		}
	}
}

void AArenaCharacter::OnReload()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed() && bInCombat == true)
	{
		IdleTime = 0.0f;
		if (CurrentWeapon)
		{
			CurrentWeapon->StartReload();
		}
	}
}

void AArenaCharacter::OnMelee()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed() && bInCombat ==  true)
	{
		IdleTime = 0.0f;
		if (CurrentWeapon)
		{
			CurrentWeapon->StartMelee();
		}
	}
}

void AArenaCharacter::OnDodge()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		IdleTime = 0.0f;

		bPressedDodgeForward = false;
		bPressedDodgeBack = false;
		bPressedDodgeLeft = false;
		bPressedDodgeRight = false;

		if (MovementStrafeAxis > 0.5f)
		{
			bPressedDodgeRight = true;
		}
		else if (MovementStrafeAxis < -0.5f)
		{
			bPressedDodgeLeft = true;
		}
		else if (MovementForwardAxis >= 0.f)
		{
			bPressedDodgeForward = true;
		}
		else
		{
			bPressedDodgeBack = true;
		}
		//StartDodge();
	}
}

void AArenaCharacter::OnThrow()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed() && PlayerConfig.Energy >= 500 && bInCombat == true)
	{
		IdleTime = 0.0f;
		StartThrow();
	}
}

void AArenaCharacter::OnStartJump()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if ((MyPC && MyPC->IsGameInputAllowed()) && (!GetCharacterMovement()->IsFalling()))
	{
		IdleTime = 0.0f;
		if (this->PlayerConfig.Stamina >= JumpCost)
		{	
			SetRunning(false, false);
			SetCrouched(false, false);
			if (Role < ROLE_Authority)
			{
				ServerJump(this);
			}
			else
			{
				this->PlayerConfig.Stamina -= JumpCost;
			}
			bPressedJump = true;
		}
	}
}

void AArenaCharacter::OnStopJump()
{
	bWantsToJump = false;
	bPressedJump = false;
}

void AArenaCharacter::OnStartCrouching()
{
	if (bInCombat == true)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchedMovementSpeed;
		IdleTime = 0.0f;
		SetRunning(false, false);
		SetCrouched(true, false);
		Crouch();
	}
}

void AArenaCharacter::OnStopCrouching()
{
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

	SetCrouched(false, false);
	UnCrouch();
}

void AArenaCharacter::OnStartRunning()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed() && !GetCharacterMovement()->IsFalling() && !GetVelocity().IsZero() && PlayerConfig.Stamina > SprintCost && bInCombat == true)
	{
		IdleTime = 0.0f;
		if (IsTargeting())
		{
			SetTargeting(false);
		}

		GetCharacterMovement()->MaxWalkSpeed = RunningMovementSpeed;

		StopWeaponFire();
		SetRunning(true, false);
	}
}

void AArenaCharacter::OnStopRunning()
{
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	SetRunning(false, false);
	// move a little bit
	//TimeLine->SetTimelineLength = 0.5f;
	//TimeLine->CallFunction = MoveForward(400.0f);
}

//////////////////////////////////////////////////////////////////////////
// Reading data

float AArenaCharacter::GetBaseMovementSpeed() const
{
	return BaseMovementSpeed;
}

USkeletalMeshComponent* AArenaCharacter::GetPawnMesh() const
{
	return GetMesh();
}

AArenaRangedWeapon* AArenaCharacter::GetWeapon() const
{
	return CurrentWeapon;
}

FName AArenaCharacter::GetWeaponAttachPoint() const
{
	return WeaponAttachPoint;
}

FName AArenaCharacter::GetOffHandAttachPoint() const
{
	return OffHandAttachPoint;
}

FName AArenaCharacter::GetOffWeaponAttachPoint() const
{
	return OffWeaponAttachPoint;
}

FName AArenaCharacter::GetMainWeaponAttachPoint() const
{
	return MainWeaponAttachPoint;
}

int32 AArenaCharacter::GetInventoryCount() const
{
	return Inventory.Num();
}

AArenaRangedWeapon* AArenaCharacter::GetInventoryWeapon(int32 index) const
{
	return Inventory[index];
}

float AArenaCharacter::GetTargetingMovementSpeed() const
{
	return TargetingMovementSpeed;
}

bool AArenaCharacter::IsTargeting() const
{
	return bIsTargeting;
}

bool AArenaCharacter::IsFiring() const
{
	return bWantsToFire;
};

float AArenaCharacter::GetRunningMovementSpeed() const
{
	return RunningMovementSpeed;
}

float AArenaCharacter::GetCrouchedMovementSpeed() const
{
	return CrouchedMovementSpeed;
}

bool AArenaCharacter::IsRunning() const
{
	if (!GetCharacterMovement())
	{
		return false;
	}
	return (bWantsToRun) && !GetVelocity().IsZero() && (GetVelocity().SafeNormal2D() | GetActorRotation().Vector()) > -0.1;
}

bool AArenaCharacter::IsThrowing() const
{
	return bWantsToThrow;
}

int32 AArenaCharacter::GetMaxHealth() const
{
	return GetClass()->GetDefaultObject<AArenaCharacter>()->PlayerConfig.Health;
}

float AArenaCharacter::GetCurrentHealth() const
{
	return PlayerConfig.Health;
}

int32 AArenaCharacter::GetMaxStamina() const
{
	return GetClass()->GetDefaultObject<AArenaCharacter>()->PlayerConfig.Stamina;
}

float AArenaCharacter::GetCurrentStamina() const
{
	return PlayerConfig.Stamina;
}

int32 AArenaCharacter::GetMaxEnergy() const
{
	return GetClass()->GetDefaultObject<AArenaCharacter>()->PlayerConfig.Energy;
}

float AArenaCharacter::GetCurrentEnergy() const
{
	return PlayerConfig.Energy;
}

bool AArenaCharacter::IsAlive() const
{
	return PlayerConfig.Health > 0;
}

float AArenaCharacter::GetLowHealthPercentage() const
{
	return PlayerConfig.LowHealthPercentage;
}

float AArenaCharacter::GetIdleTime() const
{
	return IdleTime;
}

bool AArenaCharacter::GetCombat() const
{
	return bInCombat;
}

void AArenaCharacter::UpdateTeamColorsAllMIDs()
{
	for (int32 i = 0; i < MeshMIDs.Num(); ++i)
	{
		UpdateTeamColors(MeshMIDs[i]);
	}
}

void AArenaCharacter::UpdateRunSounds(bool bNewRunning)
{
	if (bNewRunning)
	{
		if (!RunLoopAC && RunLoopSound)
		{
			RunLoopAC = UGameplayStatics::PlaySoundAttached(RunLoopSound, GetRootComponent());
			if (RunLoopAC)
			{
				RunLoopAC->bAutoDestroy = false;
			}
		}
		else if (RunLoopAC)
		{
			RunLoopAC->Play();
		}
	}
	else
	{
		if (RunLoopAC)
		{
			RunLoopAC->Stop();
		}
		if (RunStopSound)
		{
			UGameplayStatics::PlaySoundAttached(RunStopSound, GetRootComponent());
		}
	}
}

void AArenaCharacter::UpdatePawnMeshes()
{
	//Mesh->MeshComponentUpdateFlag = bFirstPerson ? EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered : EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;
	//Mesh->SetOwnerNoSee(bFirstPerson);
}

void AArenaCharacter::UpdateTeamColors(UMaterialInstanceDynamic* UseMID)
{
	if (UseMID)
	{
		/*
		AArenaPlayerState* MyPlayerState = Cast<AArenaPlayerState>(PlayerState);
		if (MyPlayerState != NULL)
		{
		float MaterialParam = (float)MyPlayerState->GetTeamNum();
		UseMID->SetScalarParameterValue(TEXT("Team Color Index"), MaterialParam);
		}
		*/
	}
}

//Pawn::PlayDying sets this lifespan, but when that function is called on client, dead pawn's role is still SimulatedProxy despite bTearOff being true. 
void AArenaCharacter::TornOff()
{
	SetLifeSpan(25.f);
}

//////////////////////////////////////////////////////////////////////////
// Damage & death

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

float AArenaCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	IdleTime = 0.0f;
	if (PlayerConfig.Health <= 0.f)
	{
		return 0.f;
	}

	// Modify based on game rules.
	//AArenaGameMode* const Game = GetWorld()->GetAuthGameMode<AArenaGameMode>();
	//Damage = Game ? Game->ModifyDamage(Damage, this, DamageEvent, EventInstigator, DamageCauser) : 0.f;

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		PlayerConfig.Health -= ActualDamage;
		if (PlayerConfig.Health <= 0)
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

void AArenaCharacter::Suicide()
{
	KilledBy(this);
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

		Die(PlayerConfig.Health, FDamageEvent(UDamageType::StaticClass()), Killer, NULL);
	}
}

bool AArenaCharacter::CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const
{
	if (bIsDying										// already dying
		|| IsPendingKill()								// already destroyed
		|| Role != ROLE_Authority						// not authority
		|| GetWorld()->GetAuthGameMode() == NULL
		|| GetWorld()->GetAuthGameMode()->GetMatchState() == MatchState::LeavingMap)	// level transition occurring
	{
		return false;
	}

	return true;
}

bool AArenaCharacter::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{
	if (!CanDie(KillingDamage, DamageEvent, Killer, DamageCauser))
	{
		return false;
	}

	PlayerConfig.Health = FMath::Min(0.0f, PlayerConfig.Health);

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

void AArenaCharacter::FellOutOfWorld(const class UDamageType& dmgType)
{
	Die(PlayerConfig.Health, FDamageEvent(dmgType.GetClass()), NULL, NULL);
}

void AArenaCharacter::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	// Only replicate this property for a short duration after it changes so join in progress players don't get spammed with fx when joining late
	DOREPLIFETIME_ACTIVE_OVERRIDE(AArenaCharacter, LastTakeHitInfo, GetWorld() && GetWorld()->GetTimeSeconds() < LastTakeHitTimeTimeout);
}

void AArenaCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	if (bIsDying)
	{
		return;
	}

	bReplicateMovement = false;
	bTearOff = true;
	bIsDying = true;

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
	DestroyInventory();

	// switch back to 3rd person view
	UpdatePawnMeshes();

	DetachFromControllerPendingDestroy();
	StopAllAnimMontages();

	if (LowHealthWarningPlayer && LowHealthWarningPlayer->IsPlaying())
	{
		LowHealthWarningPlayer->Stop();
	}

	if (RunLoopAC)
	{
		RunLoopAC->Stop();
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

void AArenaCharacter::SetRagdollPhysics()
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

//////////////////////////////////////////////////////////////////////////
// Inventory

void AArenaCharacter::SetCurrentWeapon(class AArenaRangedWeapon* NextWeapon, class AArenaRangedWeapon* PrevWeapon)
{
	if (PrevWeapon != NULL)
	{
		LastWeapon = PrevWeapon;
	}

	NewWeapon = NextWeapon;
}

void AArenaCharacter::StartEquipWeapon()
{
	if (LastWeapon)
	{
		if (LastWeapon->GetIsPrimaryWeapon() == true)
		{
			LastWeapon->SetOwningPawn(this);
			LastWeapon->OnUnEquip(true);
		}
		else
		{
			LastWeapon->SetOwningPawn(this);
			LastWeapon->OnUnEquip(false);
		}
	}
}

void AArenaCharacter::FinishEquipWeapon()
{
	CurrentWeapon = NewWeapon;

	if (CurrentWeapon->GetIsPrimaryWeapon() == true)
	{
		NewWeapon->SetOwningPawn(this);	// Make sure weapon's MyPawn is pointing back to us. During replication, we can't guarantee APawn::CurrentWeapon will rep after AWeapon::MyPawn!
		NewWeapon->OnEquip(true);
	}
	else
	{
		NewWeapon->SetOwningPawn(this);	// Make sure weapon's MyPawn is pointing back to us. During replication, we can't guarantee APawn::CurrentWeapon will rep after AWeapon::MyPawn!
		NewWeapon->OnEquip(false);
	}
}

void AArenaCharacter::OnRep_Throw()
{
	if (bWantsToThrow)
	{
		StartThrow(true);
	}
	else
	{
		StopThrow();
	}
}

void AArenaCharacter::OnRep_CurrentWeapon(AArenaRangedWeapon* LastWeapon)//recall
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
	StartEquipWeapon();
	GetWorldTimerManager().SetTimer(this, &AArenaCharacter::FinishEquipWeapon, 1.5f, false);

}

void AArenaCharacter::OnRep_PrimaryWeapon(AArenaRangedWeapon* NewWeapon)
{
	PrimaryWeapon->SetIsPrimaryWeapon(true);
	if (PrimaryWeapon == CurrentWeapon)
	{
		SetCurrentWeapon(PrimaryWeapon, LastWeapon);
		StartEquipWeapon();
		GetWorldTimerManager().SetTimer(this, &AArenaCharacter::FinishEquipWeapon, 1.5f, false);
	}
	else
	{
		PrimaryWeapon->SetOwningPawn(this);
		PrimaryWeapon->OnUnEquip(true);
	}
}

void AArenaCharacter::OnRep_SecondaryWeapon(AArenaRangedWeapon* NewWeapon)
{
	if (SecondaryWeapon == CurrentWeapon)
	{
		SetCurrentWeapon(SecondaryWeapon, LastWeapon);
		StartEquipWeapon();
		GetWorldTimerManager().SetTimer(this, &AArenaCharacter::FinishEquipWeapon, 1.5f, false);
	}
	else
	{
		SecondaryWeapon->SetOwningPawn(this);
		SecondaryWeapon->OnUnEquip(false);
	}
}

void AArenaCharacter::OnRep_CombatState(bool bOldCombatState)
{
	if (bOldCombatState == false)
	{
		this->bInCombat = true;
	}
	else
	{
		this->bInCombat = false;
	}

	UpdateCombatState();
}

void AArenaCharacter::SpawnDefaultInventory()
{
	if (Role < ROLE_Authority)
	{
		return;
	}

	int32 NumWeaponClasses = 2; //DefaultInventoryClasses.Num();
	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (DefaultInventoryClasses[i])
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.bNoCollisionFail = true;
			AArenaRangedWeapon* NewWeapon = GetWorld()->SpawnActor<AArenaRangedWeapon>(DefaultInventoryClasses[i], SpawnInfo);
			AddWeapon(NewWeapon);
		}
	}

	// equip first weapon in inventory
	if (Inventory.Num() > 0)
	{
		PrimaryWeapon = Inventory[0];
		PrimaryWeapon->SetIsPrimaryWeapon(true);
		SecondaryWeapon = Inventory[1];
		InitializeWeapons(PrimaryWeapon, SecondaryWeapon);
	}
}

void AArenaCharacter::DestroyInventory()
{
	if (Role < ROLE_Authority)
	{
		return;
	}

	// remove all weapons from inventory and destroy them
	for (int32 i = Inventory.Num() - 1; i >= 0; i--)
	{
		AArenaRangedWeapon* Weapon = Inventory[i];
		if (Weapon)
		{
			RemoveWeapon(Weapon);
			Weapon->Destroy();
		}
	}
}


bool AArenaCharacter::ServerEquipWeapon_Validate(AArenaRangedWeapon* ToEquip, AArenaRangedWeapon* ToUnEquip, bool IsEnteringCombat)
{
	return true;
}

void AArenaCharacter::ServerEquipWeapon_Implementation(AArenaRangedWeapon* ToEquip, AArenaRangedWeapon* ToUnEquip, bool IsEnteringCombat)
{
	EquipWeapon(ToEquip, ToUnEquip, IsEnteringCombat);
}

bool AArenaCharacter::ServerInitializeWeapons_Validate(AArenaRangedWeapon* mainWeapon, AArenaRangedWeapon* offWeapon)
{
	return true;
}

void AArenaCharacter::ServerInitializeWeapons_Implementation(AArenaRangedWeapon* mainWeapon, AArenaRangedWeapon* offWeapon)
{
	InitializeWeapons(mainWeapon, offWeapon);
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
	SetRunning(bNewRunning, bToggle);
}

bool AArenaCharacter::ServerSetCrouched_Validate(bool bNewCrouched, bool bToggle)
{
	return true;
}

void AArenaCharacter::ServerSetCrouched_Implementation(bool bNewCrouched, bool bToggle)
{
	SetCrouched(bNewCrouched, bToggle);
}

bool AArenaCharacter::ServerJump_Validate(class AArenaCharacter* client)
{
	return true;
}

void AArenaCharacter::ServerJump_Implementation(class AArenaCharacter* client)
{
	client->PlayerConfig.Stamina -= JumpCost;
}

bool AArenaCharacter::ServerStartThrow_Validate()
{
	return true;
}

void AArenaCharacter::ServerStartThrow_Implementation()
{
	StartThrow();
}

bool AArenaCharacter::ServerStopThrow_Validate()
{
	return true;
}

void AArenaCharacter::ServerStopThrow_Implementation()
{
	StopThrow();
}

bool AArenaCharacter::ServerIdleTimer_Validate(const float idleTimer, class AArenaCharacter* client)
{
	return true;
}

void AArenaCharacter::ServerIdleTimer_Implementation(const float idleTimer, class AArenaCharacter* client)
{
	client->IdleTime = idleTimer;
}

bool AArenaCharacter::ServerSetCombat_Validate(bool bNewCombatState)
{
	return true;
}

void AArenaCharacter::ServerSetCombat_Implementation(bool bNewCombatState)
{
	bInCombat = bNewCombatState;

	UpdateCombatState();
}


void AArenaCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// only to local owner: weapon change requests are locally instigated, other clients don't need it
	DOREPLIFETIME_CONDITION(AArenaCharacter, Inventory, COND_OwnerOnly);

	// everyone except local owner: flag change is locally instigated
	DOREPLIFETIME_CONDITION(AArenaCharacter, bIsTargeting, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AArenaCharacter, bWantsToRun, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AArenaCharacter, bWantsToCrouch, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AArenaCharacter, bWantsToThrow, COND_Custom);
	DOREPLIFETIME_CONDITION(AArenaCharacter, LastTakeHitInfo, COND_Custom);

	// everyone
	DOREPLIFETIME(AArenaCharacter, IdleTime);
	DOREPLIFETIME(AArenaCharacter, bInCombat);
	DOREPLIFETIME(AArenaCharacter, CurrentWeapon);
	DOREPLIFETIME(AArenaCharacter, PrimaryWeapon);
	DOREPLIFETIME(AArenaCharacter, SecondaryWeapon);
	DOREPLIFETIME(AArenaCharacter, PlayerConfig);
}
