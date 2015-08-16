// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "TheArena.h"

//////////////////////////////////////////////////////////////////////////
// AArenaCharacter

AArenaCharacter::AArenaCharacter(const class FObjectInitializer& PCIP)
	: Super(PCIP.SetDefaultSubobjectClass<UArenaCharacterMovement>(ACharacter::CharacterMovementComponentName))
{
	CharacterState = PCIP.CreateDefaultSubobject<UArenaCharacterState>(this, TEXT("CharacterState"));
	CharacterAttributes = PCIP.CreateDefaultSubobject<UArenaCharacterAttributes>(this, TEXT("CharacterAttributes"));
	CharacterEquipment = PCIP.CreateDefaultSubobject<UArenaCharacterEquipment>(this, TEXT("CharacterEquipment"));
	CharacterInventory = PCIP.CreateDefaultSubobject<UArenaCharacterInventory>(this, TEXT("CharacterInventory"));
	SaveGameInstance = Cast<UArenaSaveGame>(UGameplayStatics::CreateSaveGameObject(UArenaSaveGame::StaticClass()));
	CharacterMovementComponent = Cast<UArenaCharacterMovement>(GetCharacterMovement());

	//CharacterState->SetNetAddressable();
	CharacterState->SetIsReplicated(true);

	CharacterAttributes->SetNetAddressable();
	CharacterAttributes->SetIsReplicated(true);

	//CharacterEquipment->SetNetAddressable();
	CharacterEquipment->SetIsReplicated(true);

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
	//FollowCamera->scale

	Busy = false;
	Spawned = false;
	ReadySpawned = false;
	ActionQueue = 0.0f;
}

void AArenaCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CharacterState->SetMyPawn(this);
	CharacterEquipment->SetMyPawn(this);

	//GetWorldTimerManager().SetTimer(this, &AArenaCharacter::LoadPersistence, 0.25f, false);

	//LoadPersistence();
	if (IsRunningGame() || IsRunningDedicatedServer())
	{
		ReadySpawned = true;
		//GetWorldTimerManager().SetTimer(this, &AArenaCharacter::LoadPersistence, 0.5f, false);
	}
	else
	{
		ReadySpawned = true;
		//GetWorldTimerManager().SetTimer(this, &AArenaCharacter::LoadPersistence, 0.5f, false);
		//ServerSpawnEquipment(CharacterEquipment->GetPrimaryWeaponBP(), CharacterEquipment->GetSecondaryWeaponBP(), CharacterEquipment->GetUpperBackUtilityBP());
	}

	CharacterState->Reset();
	UpdatePawnMeshes();

	// create material instance for setting team colors (3rd person view)
	for (int32 iMat = 0; iMat < GetMesh()->GetNumMaterials(); iMat++)
	{
		MeshMIDs.Add(GetMesh()->CreateAndSetMaterialInstanceDynamic(iMat));
	}

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

void AArenaCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (CurrentWeapon)
	{
		CurrentWeapon->Equip();
	}
}

void AArenaCharacter::LoadPersistence()
{
	AArenaPlayerState* MyPlayerState = Cast<AArenaPlayerState>(PlayerState);
	if (PlayerState)
	{
		Name = MyPlayerState->PlayerName;
	}

	//if (Role == ROLE_Authority)
	//{
	//	SetName(Name);
	//}
	//else
	//{
	//	ServerSetName(Name);
	//}

	if (IsLocallyControlled())
	{
		SaveGameInstance = Cast<UArenaSaveGame>(UGameplayStatics::LoadGameFromSlot(Name, SaveGameInstance->UserIndex));
		if (SaveGameInstance)
		{
			CharacterEquipment->SetPrimaryWeaponBP(SaveGameInstance->PrimaryWeapon);
			CharacterEquipment->SetSecondaryWeaponBP(SaveGameInstance->SecondaryWeapon);
			CharacterEquipment->SetHeadUtilityBP(SaveGameInstance->HeadUtility);
			CharacterEquipment->SetUpperBackUtilityBP(SaveGameInstance->UpperBackUtility);
			CharacterEquipment->SetLowerBackUtilityBP(SaveGameInstance->LowerBackUtility);
			CharacterEquipment->SetLeftWristUtilityBP(SaveGameInstance->LeftWristUtility);
			CharacterEquipment->SetRightWristUtilityBP(SaveGameInstance->RightWristUtility);
			CharacterEquipment->SetLeftWaistUtilityBP(SaveGameInstance->LeftWaistUtility);
			CharacterEquipment->SetRightWaistUtilityBP(SaveGameInstance->RightWaistUtility);
		}

		ServerSpawnEquipment(CharacterEquipment->GetPrimaryWeaponBP(), CharacterEquipment->GetSecondaryWeaponBP(), CharacterEquipment->GetUpperBackUtilityBP());

		SaveGameInstance = Cast<UArenaSaveGame>(UGameplayStatics::CreateSaveGameObject(UArenaSaveGame::StaticClass()));
		SaveGameInstance->PrimaryWeapon = CharacterEquipment->GetPrimaryWeaponBP();
		SaveGameInstance->SecondaryWeapon = CharacterEquipment->GetSecondaryWeaponBP();
		SaveGameInstance->HeadUtility = CharacterEquipment->GetHeadUtilityBP();
		SaveGameInstance->UpperBackUtility = CharacterEquipment->GetUpperBackUtilityBP();
		SaveGameInstance->LowerBackUtility = CharacterEquipment->GetLowerBackUtilityBP();
		SaveGameInstance->LeftWristUtility = CharacterEquipment->GetLeftWristUtilityBP();
		SaveGameInstance->RightWristUtility = CharacterEquipment->GetRightWristUtilityBP();
		SaveGameInstance->LeftWaistUtility = CharacterEquipment->GetLeftWaistUtilityBP();
		SaveGameInstance->RightWaistUtility = CharacterEquipment->GetRightWaistUtilityBP();
		UGameplayStatics::SaveGameToSlot(SaveGameInstance, Name, SaveGameInstance->UserIndex);
	}
	Spawned = true;
}

////////////////////////////////////////// Character Defaults //////////////////////////////////////////

void AArenaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (MyPC)
	{
		CharacterAttributes->Regenerate(DeltaSeconds);
		CharacterMovementComponent->ManageState(DeltaSeconds);
	}

	if (!Spawned && ReadySpawned)
	{
		LoadPersistence();
	}

	AArenaPlayerState* MyPlayerState = Cast<AArenaPlayerState>(PlayerState);
	if (MyPlayerState)
	{
		MyPlayerState->MyPawn = this;
	}
}

void AArenaCharacter::Destroyed()
{
	Super::Destroyed();
	DestroyInventory();
}

void AArenaCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	UpdatePawnMeshes();

	// set team colors for 1st person view
	UMaterialInstanceDynamic* Mesh1PMID = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
	UpdateTeamColors(Mesh1PMID);
}

void AArenaCharacter::PossessedBy(class AController* InController)
{
	Super::PossessedBy(InController);

	// [server] as soon as PlayerState is assigned, set team colors of this pawn for local player
	UpdateTeamColorsAllMIDs();
}

void AArenaCharacter::UpdateTeamColorsAllMIDs()
{
	for (int32 i = 0; i < MeshMIDs.Num(); ++i)
	{
		UpdateTeamColors(MeshMIDs[i]);
	}
}

void AArenaCharacter::UpdateTeamColors(UMaterialInstanceDynamic* UseMID)
{
	if (UseMID)
	{
		AArenaPlayerState* MyPlayerState = Cast<AArenaPlayerState>(PlayerState);
		if (MyPlayerState != NULL)
		{
			float MaterialParam = (float)MyPlayerState->GetTeamNum();
			UseMID->SetScalarParameterValue(TEXT("Team Color Index"), MaterialParam);
		}
	}
}

FRotator AArenaCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}

bool AArenaCharacter::GetBusy()
{
	return Busy;
}
void AArenaCharacter::ToggleBusy_Implementation()
{
	if (Busy)
	{
		Busy = false;
		ActionQueue = 0.0f;
	}
	else
	{
		Busy = true;
	}
}

void AArenaCharacter::SetLocation_Implementation()
{
	if (CharacterState->GetPlayerState() == EPlayerState::Covering
		&& (CharacterState->GetCoverState() == ECoverState::HighLeft
		|| CharacterState->GetCoverState() == ECoverState::HighRight
		|| CharacterState->GetCoverState() == ECoverState::LowLeft
		|| CharacterState->GetCoverState() == ECoverState::LowRight))
	{
		SetActorLocation(CharacterMovementComponent->GetLocation());
		CurrentWeapon->GetWeaponState()->SetCoverTargeting(false);
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

	InputComponent->BindAction("Vault", IE_Pressed, this, &AArenaCharacter::OnVault);

	InputComponent->BindAction("Climb", IE_Pressed, this, &AArenaCharacter::OnClimb);

	InputComponent->BindAction("Crouch", IE_Pressed, this, &AArenaCharacter::OnCrouch);

	InputComponent->BindAction("Cover", IE_Pressed, this, &AArenaCharacter::OnToggleCover);

	InputComponent->BindAction("ExitCover", IE_Pressed, this, &AArenaCharacter::OnExitCover);

	InputComponent->BindAction("Sprint", IE_Pressed, this, &AArenaCharacter::OnStartRunning);
	InputComponent->BindAction("Sprint", IE_Released, this, &AArenaCharacter::OnStopRunning);

	InputComponent->BindAction("Targeting", IE_Pressed, this, &AArenaCharacter::OnStartTargeting);
	InputComponent->BindAction("Targeting", IE_Released, this, &AArenaCharacter::OnStopTargeting);

	InputComponent->BindAction("Peaking", IE_Pressed, this, &AArenaCharacter::OnStartPeaking);
	InputComponent->BindAction("Peaking", IE_Released, this, &AArenaCharacter::OnStopPeaking);

	InputComponent->BindAction("ReadyWeapon", IE_Pressed, this, &AArenaCharacter::OnToggleCombat);

	InputComponent->BindAction("SwapWeapon", IE_Pressed, this, &AArenaCharacter::OnSwapWeapon);

	InputComponent->BindAction("Reload", IE_Pressed, this, &AArenaCharacter::OnReload);

	InputComponent->BindAction("Melee", IE_Pressed, this, &AArenaCharacter::OnMelee);

	InputComponent->BindAction("Dodge", IE_Pressed, this, &AArenaCharacter::OnDodge);

	InputComponent->BindAction("Fire", IE_Pressed, this, &AArenaCharacter::OnStartFire);
	InputComponent->BindAction("Fire", IE_Released, this, &AArenaCharacter::OnStopFire);

	InputComponent->BindAction("Back", IE_Pressed, this, &AArenaCharacter::OnActivateBack);
	InputComponent->BindAction("Back", IE_Released, this, &AArenaCharacter::OnDeactivateBack);
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
	if (ArenaCharacterCan::MoveRight(this, MyPC, Value))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		if (CharacterState->GetPlayerState() == EPlayerState::Covering)
		{
			AddMovementInput(CharacterMovementComponent->GetCoverDirection(), Value);
		}
		else
		{
			AddMovementInput(Direction, Value);
		}
	}
	if (Value > 0)
	{
		CharacterMovementComponent->SetDirection(FName(TEXT("Right")));
	}
	if (Value < 0)
	{
		CharacterMovementComponent->SetDirection(FName(TEXT("Left")));
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
		UnCrouch();
		OnStopRunning();
		CharacterState->SetPlayerState(EPlayerState::Jumping);
		CharacterAttributes->SetCurrentStamina(CharacterAttributes->GetCurrentStamina() - CharacterMovementComponent->CostConfig.JumpCost);
		Jump();
	}
}
void AArenaCharacter::OnStopJump()
{
	if (CharacterState->GetPlayerState() == EPlayerState::Jumping)
	{
		CharacterState->SetPlayerState(EPlayerState::Default);
	}
}
void AArenaCharacter::OnVault()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Vault(this, MyPC))
	{
		CharacterAttributes->SetCurrentStamina(CharacterAttributes->GetCurrentStamina() - CharacterMovementComponent->CostConfig.VaultCost);
		if (Role == ROLE_Authority)
		{
			StartVault();
		}
		else
		{
			ServerVault();
		}
	}
}
void AArenaCharacter::OnClimb()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Climb(this, MyPC))
	{
		CharacterAttributes->SetCurrentStamina(CharacterAttributes->GetCurrentStamina() - CharacterMovementComponent->CostConfig.ClimbCost);
		if (Role == ROLE_Authority)
		{
			StartClimb();
		}
		else
		{
			ServerClimb();
		}
	}
}
void AArenaCharacter::OnCrouch()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Crouch(this, MyPC))
	{
		if (Role == ROLE_Authority)
		{
			ToggleCrouch();
		}
		else
		{
			ServerToggleCrouch();
		}
	}
}
void AArenaCharacter::OnToggleCover()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Cover(this, MyPC))
	{
		if (Role == ROLE_Authority)
		{
			ToggleCover();
		}
		else
		{
			ServerToggleCover();
		}
	}
}
void AArenaCharacter::OnExitCover()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Cover(this, MyPC))
	{
		if (Role == ROLE_Authority)
		{
			ExitCover();
		}
		else
		{
			ServerExitCover();
		}
	}
}
void AArenaCharacter::OnStartRunning()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Run(this, MyPC))
	{
		if (Role == ROLE_Authority)
		{
			Running(true);
		}
		else
		{
			ServerRunning(true);
		}
	}
}
void AArenaCharacter::OnStopRunning()
{
	if (Role == ROLE_Authority)
	{
		Running(false);
	}
	else
	{
		ServerRunning(false);
	}
}
void AArenaCharacter::OnStartTargeting()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Target(this, MyPC))
	{
		if (Role == ROLE_Authority)
		{
			StartTargeting();
		}
		else
		{
			ServerStartTargeting();
		}
	}
}
void AArenaCharacter::OnStopTargeting()
{
	if (Role == ROLE_Authority)
	{
		StopTargeting();
	}
	else
	{
		ServerStopTargeting();
	}
}
void AArenaCharacter::OnStartPeaking()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Peak(this, MyPC))
	{
		if (Role == ROLE_Authority)
		{
			StartPeaking();
		}
		else
		{
			ServerStartPeaking();
		}
	}
}
void AArenaCharacter::OnStopPeaking()
{
	if (Peaking)
	{
		if (Role == ROLE_Authority)
		{
			if (Busy)
			{
				GetWorldTimerManager().SetTimer(this, &AArenaCharacter::StopPeaking, ActionQueue * 0.3f, false);
			}
			else
			{
				StopPeaking();
			}
		}
		else
		{
			ServerStopPeaking();
		}
	}
}
void AArenaCharacter::OnToggleCombat()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::ToggleCombat(this, MyPC))
	{
		if (Role == ROLE_Authority)
		{	
			ToggleCombat();
		}
		else
		{
			ServerEnterCombat();
		}
	}
}
void AArenaCharacter::OnSwapWeapon()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Swap(this, MyPC))
	{
		if (Role == ROLE_Authority)
		{
			SwapWeapon();
		}
		else
		{
			ServerSwapWeapon();
		}
	}
}
void AArenaCharacter::OnReload()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Reload(this, MyPC))
	{
		if (CurrentWeapon)
		{
			//CharacterEquipment->GetCurrentWeapon()->StartReload();
			CurrentWeapon->StartReload();
		}
	}
}
void AArenaCharacter::OnMelee()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Melee(this, MyPC))
	{
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

	}
}
void AArenaCharacter::OnStartFire()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Fire(this, MyPC))
	{
		CurrentWeapon->StartAttack();
	}
}
void AArenaCharacter::OnStopFire()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Fire(this, MyPC))
	{
		CurrentWeapon->StopAttack();
	}
}
void AArenaCharacter::OnActivateBack()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Back(this, MyPC))
	{
		UpperBackUtility->Activate();
	}
}
void AArenaCharacter::OnDeactivateBack()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (UpperBackUtility)
	{
		UpperBackUtility->Deactivate();
	}
}

////////////////////////////////////////// Action Functions //////////////////////////////////////////

void AArenaCharacter::InitializeWeapons(AArenaWeapon* mainWeapon, AArenaWeapon* offWeapon, AArenaUtility* UpperBack)
{
	if (Role == ROLE_Authority)
	{
		if (PrimaryWeapon)
		{
			PrimaryWeapon->SetOwningPawn(this);
			PrimaryWeapon->FinishUnEquip();
		}
		if (SecondaryWeapon)
		{
			SecondaryWeapon->SetOwningPawn(this);;
			SecondaryWeapon->FinishUnEquip();
		}

		if (HeadUtility)
		{
			HeadUtility->SetMyPawn(this);
		}
		if (UpperBackUtility)
		{
			UpperBackUtility->SetMyPawn(this);
			UpperBackUtility->Equip();
		}
		if (LowerBackUtility)
		{
			LowerBackUtility->SetMyPawn(this);
		}
		if (LeftWristUtility)
		{
			LeftWristUtility->SetMyPawn(this);
		}
		if (RightWristUtility)
		{
			RightWristUtility->SetMyPawn(this);
		}
		if (LeftWaistUtility)
		{
			LeftWaistUtility->SetMyPawn(this);
		}
		if (RightWaistUtility)
		{
			RightWaistUtility->SetMyPawn(this);
		}
	}
	else
	{
		ServerInitializeWeapons(mainWeapon, offWeapon, UpperBack);
	}
}

void AArenaCharacter::ToggleCrouch_Implementation()
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

void AArenaCharacter::ToggleCover_Implementation()
{
	if (CharacterState->GetPlayerState() == EPlayerState::Covering)
	{
		ExitCover();
	}
	else
	{
		UnCrouch();
		CharacterState->SetPlayerState(EPlayerState::Covering);
	}
}

void AArenaCharacter::ExitCover_Implementation()
{
	Peaking = false;
	CharacterState->SetPlayerState(EPlayerState::Default);
	CharacterState->SetCoverState(ECoverState::Default);
	CharacterState->SetIsNearLeftEdge(false);
	CharacterState->SetIsNearRightEdge(false);
	CurrentWeapon->GetWeaponState()->SetCoverTargeting(false);

	//somewhere else
	if (CharacterState->GetCoverState() == ECoverState::HighLeft)
	{

	}
	else if (CharacterState->GetCoverState() == ECoverState::HighRight)
	{

	}
	else if (CharacterState->GetCoverState() == ECoverState::LowLeft)
	{

	}
	else if (CharacterState->GetCoverState() == ECoverState::LowRight)
	{

	}
}

void AArenaCharacter::Running_Implementation(bool IsRunning)
{
	if (IsRunning)
	{
		UnCrouch();
		OnStopTargeting();
		CharacterState->SetPlayerState(EPlayerState::Running);
		UpdateRunSounds(true);
	}
	else
	{
		CharacterState->SetPlayerState(EPlayerState::Default);
		UpdateRunSounds(false);
	}
}

void AArenaCharacter::StartTargeting_Implementation()
{
	if (CurrentWeapon && CharacterEquipment)
	{
		CharacterEquipment->SetDrawCrosshair(true);
		CurrentWeapon->GetWeaponState()->SetTargetingState(ETargetingState::Targeting);
	}

	if (TargetingSound)
	{
		UGameplayStatics::PlaySoundAttached(TargetingSound, GetRootComponent());
	}
}
void AArenaCharacter::StopTargeting_Implementation()
{
	if (CurrentWeapon)
	{
		CharacterEquipment->SetDrawCrosshair(false);
		CurrentWeapon->GetWeaponState()->SetTargetingState(ETargetingState::Default);
	}

	if (CharacterState->GetPlayerState() == EPlayerState::Covering)
	{

	}
}

void AArenaCharacter::StartPeaking_Implementation()
{
	Peaking = true;
	if (CharacterState->GetCoverState() == ECoverState::HighLeft)
	{
		Busy = true;
		CurrentWeapon->GetWeaponState()->SetCoverTargeting(true);
		CharacterMovementComponent->SetLocation(GetActorLocation());
		ActionQueue = PlayAnimMontage(CharacterMovementComponent->GetHighLeftAnimation(FString(TEXT("Start"))));
	}
	else if (CharacterState->GetCoverState() == ECoverState::HighRight)
	{
		Busy = true;
		CurrentWeapon->GetWeaponState()->SetCoverTargeting(true);
		CharacterMovementComponent->SetLocation(GetActorLocation());
		ActionQueue = PlayAnimMontage(CharacterMovementComponent->GetHighRightAnimation(FString(TEXT("Start"))));
	}
	else if (CharacterState->GetCoverState() == ECoverState::LowLeft)
	{
		Busy = true;
		CurrentWeapon->GetWeaponState()->SetCoverTargeting(true);
		CharacterMovementComponent->SetLocation(GetActorLocation());
		ActionQueue = PlayAnimMontage(CharacterMovementComponent->GetLowLeftAnimation(FString(TEXT("Start"))));
	}
	else if (CharacterState->GetCoverState() == ECoverState::LowRight)
	{
		Busy = true;
		CurrentWeapon->GetWeaponState()->SetCoverTargeting(true);
		CharacterMovementComponent->SetLocation(GetActorLocation());
		ActionQueue = PlayAnimMontage(CharacterMovementComponent->GetLowRightAnimation(FString(TEXT("Start"))));
	}
	else
	{
		return;
	}
	GetWorldTimerManager().SetTimer(this, &AArenaCharacter::ToggleBusy, ActionQueue * 0.40f, false);
}
void AArenaCharacter::StopPeaking_Implementation()
{
	Peaking = false;
	CurrentWeapon->StopAttack();
	if (CharacterState->GetCoverState() == ECoverState::HighLeft)
	{
		Busy = true;
		ActionQueue = PlayAnimMontage(CharacterMovementComponent->GetHighLeftAnimation(FString(TEXT("End"))));
	}
	else if (CharacterState->GetCoverState() == ECoverState::HighRight)
	{
		Busy = true;
		ActionQueue = PlayAnimMontage(CharacterMovementComponent->GetHighRightAnimation(FString(TEXT("End"))));
	}
	else if (CharacterState->GetCoverState() == ECoverState::LowLeft)
	{
		Busy = true;
		ActionQueue = PlayAnimMontage(CharacterMovementComponent->GetLowLeftAnimation(FString(TEXT("End"))));
	}
	else if (CharacterState->GetCoverState() == ECoverState::LowRight)
	{
		Busy = true;
		ActionQueue = PlayAnimMontage(CharacterMovementComponent->GetLowRightAnimation(FString(TEXT("End"))));
	}
	else
	{
		return;
	}
	GetWorldTimerManager().SetTimer(this, &AArenaCharacter::SetLocation, ActionQueue * 0.4f, false);
	GetWorldTimerManager().SetTimer(this, &AArenaCharacter::ToggleBusy, ActionQueue * 0.4f, false);
}

void AArenaCharacter::ToggleCombat_Implementation()
{
	if (CharacterState->GetCombatState() == ECombatState::Passive)
	{
		CharacterState->SetCombatState(ECombatState::Aggressive);
		GetCharacterMovement()->bOrientRotationToMovement = false;
		SetCurrentWeapon();
		EquipWeapon();
		//CharacterEquipment->GetCurrentWeapon()->SetRole(Role);
	}
	else
	{
		CharacterState->SetCombatState(ECombatState::Passive);
		GetCharacterMovement()->bOrientRotationToMovement = true;
		UnEquipWeapon();
		OnDeactivateBack();
	}
}

void AArenaCharacter::SwapWeapon_Implementation()
{
	float Duration = UnEquipWeapon();
	SetCurrentWeapon();
	GetWorldTimerManager().SetTimer(TimerHandle_SwapWeapon, this, &AArenaCharacter::EquipWeapon, Duration * 0.75f, false);
}

void AArenaCharacter::EquipWeapon()
{
	//if (Role == ROLE_Authority)
	//{
	//SetCurrentWeapon();	
	FinishEquipWeapon(CurrentWeapon);
	//}
	//else
	//{
	//	ServerEquipWeapon();
	//}
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
	return FinishUnEquipWeapon(CurrentWeapon);
}
float AArenaCharacter::FinishUnEquipWeapon(class AArenaWeapon* Weapon)
{
	float Duration = 0.0f;
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

void AArenaCharacter::StartVault_Implementation()
{
	OnStopFire();
	CharacterState->SetPlayerState(EPlayerState::Vaulting);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

	float AnimDuration = PlayAnimMontage(CharacterMovementComponent->GetVaultAnimation());
	if (AnimDuration <= 0.0f)
	{
		AnimDuration = 0.3f;
	}

	GetWorldTimerManager().SetTimer(this, &AArenaCharacter::StopVault, AnimDuration * 0.9f, false);
}
void AArenaCharacter::StopVault_Implementation()
{
	CharacterState->SetPlayerState(EPlayerState::Default);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	StopAnimMontage(CharacterMovementComponent->GetVaultAnimation());
}

void AArenaCharacter::StartClimb_Implementation()
{
	OnStopFire();
	CharacterState->SetPlayerState(EPlayerState::Climbing);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

	float AnimDuration = PlayAnimMontage(CharacterMovementComponent->GetClimbAnimation());
	if (AnimDuration <= 0.0f)
	{
		AnimDuration = 0.3f;
	}

	GetWorldTimerManager().SetTimer(this, &AArenaCharacter::StopClimb, AnimDuration * 0.9f, false);
}
void AArenaCharacter::StopClimb_Implementation()
{
	CharacterState->SetPlayerState(EPlayerState::Default);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	StopAnimMontage(CharacterMovementComponent->GetClimbAnimation());
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

UArenaCharacterInventory* AArenaCharacter::GetCharacterInventory()
{
	return CharacterInventory;
}

TArray<UMaterialInstanceDynamic*> AArenaCharacter::GetMeshMIDs()
{
	return MeshMIDs;
}

FString AArenaCharacter::GetName() const
{
	return Name;
}
void AArenaCharacter::SetName_Implementation(const FString& NewName)
{
	Name = NewName;
}

AArenaWeapon* AArenaCharacter::GetCurrentWeapon()
{
	return CurrentWeapon;
}
void AArenaCharacter::SetCurrentWeapon()
{
	if (CurrentWeapon == NULL)
	{
		CurrentWeapon = PrimaryWeapon;
		return;
	}
	if (CurrentWeapon == PrimaryWeapon)
	{
		CurrentWeapon = SecondaryWeapon;
		return;
	}
	if (CurrentWeapon == SecondaryWeapon)
	{
		CurrentWeapon = PrimaryWeapon;
		return;
	}
}

AArenaWeapon* AArenaCharacter::GetPrimaryWeapon()
{
	return PrimaryWeapon;
}
void AArenaCharacter::SetPrimaryWeapon(TSubclassOf<class AArenaWeapon> Weapon)
{
	if (IsLocallyControlled() && Weapon)
	{
		//SaveGameInstance = Cast<UArenaSaveGame>(UGameplayStatics::CreateSaveGameObject(UArenaSaveGame::StaticClass()));
		SaveGameInstance->PrimaryWeapon = Weapon;
  UGameplayStatics::SaveGameToSlot(SaveGameInstance, Name, SaveGameInstance->UserIndex);
	}
	if (Role == ROLE_Authority)
	{
		HandlePrimaryWeapon(Weapon);
	}
	else
	{
		ServerSetPrimaryWeapon(Weapon);
	}
}
void AArenaCharacter::HandlePrimaryWeapon(TSubclassOf<class AArenaWeapon> Weapon)
{
	CurrentWeapon = NULL;
	AArenaWeapon* TBD = PrimaryWeapon;
	if (TBD)
	{
		TBD->OnLeaveInventory();
		TBD->Destroy();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	PrimaryWeapon = GetWorld()->SpawnActor<AArenaWeapon>(Weapon, SpawnInfo);
	PrimaryWeapon->SetOwningPawn(this);
	PrimaryWeapon->SetPrimary(true);

	PrimaryWeapon->UnEquip();
}

AArenaWeapon* AArenaCharacter::GetSecondaryWeapon()
{
	return SecondaryWeapon;
}
void AArenaCharacter::SetSecondaryWeapon(TSubclassOf<class AArenaWeapon> Weapon)
{
	if (IsLocallyControlled() && Weapon)
	{
		//SaveGameInstance = Cast<UArenaSaveGame>(UGameplayStatics::CreateSaveGameObject(UArenaSaveGame::StaticClass()));
		SaveGameInstance->SecondaryWeapon = Weapon;
  UGameplayStatics::SaveGameToSlot(SaveGameInstance, Name, SaveGameInstance->UserIndex);
	}
	if (Role == ROLE_Authority)
	{
		HandleSecondaryWeapon(Weapon);
	}
	else
	{
		ServerSetSecondaryWeapon(Weapon);
	}
}
void AArenaCharacter::HandleSecondaryWeapon(TSubclassOf<class AArenaWeapon> Weapon)
{
	CurrentWeapon = NULL;
	AArenaWeapon* TBD = SecondaryWeapon;
	if (TBD)
	{
		TBD->OnLeaveInventory();
		TBD->Destroy();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	SecondaryWeapon = GetWorld()->SpawnActor<AArenaWeapon>(Weapon, SpawnInfo);
	SecondaryWeapon->SetOwningPawn(this);
	SecondaryWeapon->SetPrimary(false);

	SecondaryWeapon->UnEquip();
}

class AArenaUtility* AArenaCharacter::GetHeadUtility()
{
	return HeadUtility;
}
void AArenaCharacter::SetHeadUtility(TSubclassOf<class AArenaUtility> Utility)
{
	if (IsLocallyControlled() && Utility)
	{
		//SaveGameInstance = Cast<UArenaSaveGame>(UGameplayStatics::CreateSaveGameObject(UArenaSaveGame::StaticClass()));
		SaveGameInstance->HeadUtility = Utility;
  UGameplayStatics::SaveGameToSlot(SaveGameInstance, Name, SaveGameInstance->UserIndex);
	}
	if (Role == ROLE_Authority)
	{
		//HandleSecondaryWeapon(Weapon);
	}
	else
	{
		//ServerSetSecondaryWeapon(Weapon);
	}
}

class AArenaUtility* AArenaCharacter::GetUpperBackUtility()
{
	return UpperBackUtility;
}
void AArenaCharacter::SetUpperBackUtility(TSubclassOf<class AArenaUtility> Utility)
{
	if (IsLocallyControlled() && Utility)
	{
		//SaveGameInstance = Cast<UArenaSaveGame>(UGameplayStatics::CreateSaveGameObject(UArenaSaveGame::StaticClass()));

		SaveGameInstance->UpperBackUtility = Utility;
  UGameplayStatics::SaveGameToSlot(SaveGameInstance, Name, SaveGameInstance->UserIndex);
	}
	if (Role == ROLE_Authority)
	{
		HandleUpperBackUtility(Utility);
	}
	else
	{
		ServerSetUpperBackUtility(Utility);
	}
}
void AArenaCharacter::HandleUpperBackUtility(TSubclassOf<class AArenaUtility> Utility)
{
	AArenaUtility* TBD = UpperBackUtility;
	if (TBD)
	{
		TBD->UnEquip();
		TBD->Destroy();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	UpperBackUtility = GetWorld()->SpawnActor<AArenaUtility>(Utility, SpawnInfo);
	UpperBackUtility->SetMyPawn(this);

}

class AArenaUtility* AArenaCharacter::GetLowerBackUtility()
{
	return LowerBackUtility;
}
void AArenaCharacter::SetLowerBackUtility(TSubclassOf<class AArenaUtility> Utility)
{
	if (IsLocallyControlled() && Utility)
	{
		//SaveGameInstance = Cast<UArenaSaveGame>(UGameplayStatics::CreateSaveGameObject(UArenaSaveGame::StaticClass()));
		SaveGameInstance->LowerBackUtility = Utility;
  UGameplayStatics::SaveGameToSlot(SaveGameInstance, Name, SaveGameInstance->UserIndex);
	}
	if (Role == ROLE_Authority)
	{
		//HandleSecondaryWeapon(Weapon);
	}
	else
	{
		//ServerSetSecondaryWeapon(Weapon);
	}
}

class AArenaUtility* AArenaCharacter::GetLeftWristUtility()
{
	return LeftWristUtility;
}
void AArenaCharacter::SetLeftWristUtility(TSubclassOf<class AArenaUtility> Utility)
{
	if (IsLocallyControlled() && Utility)
	{
		//SaveGameInstance = Cast<UArenaSaveGame>(UGameplayStatics::CreateSaveGameObject(UArenaSaveGame::StaticClass()));
		SaveGameInstance->LeftWristUtility = Utility;
  UGameplayStatics::SaveGameToSlot(SaveGameInstance, Name, SaveGameInstance->UserIndex);
	}
	if (Role == ROLE_Authority)
	{
		//HandleSecondaryWeapon(Weapon);
	}
	else
	{
		//ServerSetSecondaryWeapon(Weapon);
	}
}

class AArenaUtility* AArenaCharacter::GetRightWristUtility()
{
	return RightWristUtility;
}
void AArenaCharacter::SetRightWristUtility(TSubclassOf<class AArenaUtility> Utility)
{
	if (IsLocallyControlled() && Utility)
	{
		//SaveGameInstance = Cast<UArenaSaveGame>(UGameplayStatics::CreateSaveGameObject(UArenaSaveGame::StaticClass()));
		SaveGameInstance->RightWristUtility = Utility;
  UGameplayStatics::SaveGameToSlot(SaveGameInstance, Name, SaveGameInstance->UserIndex);
	}
	if (Role == ROLE_Authority)
	{
		//HandleSecondaryWeapon(Weapon);
	}
	else
	{
		//ServerSetSecondaryWeapon(Weapon);
	}
}

class AArenaUtility* AArenaCharacter::GetLeftWaistUtility()
{
	return LeftWaistUtility;
}
void AArenaCharacter::SetLeftWaistUtility(TSubclassOf<class AArenaUtility> Utility)
{
	if (IsLocallyControlled() && Utility)
	{
		//SaveGameInstance = Cast<UArenaSaveGame>(UGameplayStatics::CreateSaveGameObject(UArenaSaveGame::StaticClass()));
		SaveGameInstance->LeftWaistUtility = Utility;
  UGameplayStatics::SaveGameToSlot(SaveGameInstance, Name, SaveGameInstance->UserIndex);
	}
	if (Role == ROLE_Authority)
	{
		//HandleSecondaryWeapon(Weapon);
	}
	else
	{
		//ServerSetSecondaryWeapon(Weapon);
	}
}

class AArenaUtility*AArenaCharacter::GetRightWaistUtility()
{
	return RightWaistUtility;
}
void AArenaCharacter::SetRightWaistUtility(TSubclassOf<class AArenaUtility> Utility)
{
	if (IsLocallyControlled() && Utility)
	{
		//SaveGameInstance = Cast<UArenaSaveGame>(UGameplayStatics::CreateSaveGameObject(UArenaSaveGame::StaticClass()));
		SaveGameInstance->RightWaistUtility = Utility;
  UGameplayStatics::SaveGameToSlot(SaveGameInstance, Name, SaveGameInstance->UserIndex);
	}
	if (Role == ROLE_Authority)
	{
		//HandleSecondaryWeapon(Weapon);
	}
	else
	{
		//ServerSetSecondaryWeapon(Weapon);
	}
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
		//ApplyDamageMomentum(DamageTaken, DamageEvent, PawnInstigator, DamageCauser);
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
	if (!ArenaCharacterCan::Die(this))
	{
		return false;
	}

	CharacterAttributes->SetCurrentHealth(FMath::Min(0.0f, CharacterAttributes->GetCurrentHealth()));

	// if this is an environmental death then refer to the previous killer so that they receive credit (knocked into lava pits, etc)
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	AController* const KilledPlayer = (Controller != NULL) ? Controller : Cast<AController>(GetOwner());
	GetWorld()->GetAuthGameMode<ATheArenaGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);

	NetUpdateFrequency = GetDefault<AArenaCharacter>()->NetUpdateFrequency;
	GetCharacterMovement()->ForceReplicationUpdate();

	OnDeath(KillingDamage, DamageEvent, Killer ? Killer->GetPawn() : NULL, DamageCauser);
	return true;
}

void AArenaCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	if (CharacterAttributes->bIsDying)
	{
		return;
	}

	bReplicateMovement = false;
	bTearOff = true;
	//Spawned = false;
	CharacterAttributes->bIsDying = true;

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

void AArenaCharacter::KilledBy(APawn* EventInstigator)
{
	if (Role == ROLE_Authority && !CharacterAttributes->bIsDying)
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

void AArenaCharacter::DestroyInventory()
{
	if (Role < ROLE_Authority)
	{
		return;
	}

	AArenaWeapon* Weapon = PrimaryWeapon;
	if (Weapon)
	{
		Weapon->OnLeaveInventory();
		Weapon->Destroy();
	}

	Weapon = SecondaryWeapon;
	if (Weapon)
	{
		Weapon->OnLeaveInventory();
		Weapon->Destroy();
	}

	AArenaUtility* Utility = UpperBackUtility;
	if (Utility)
	{
		Utility->UnEquip();
		Utility->Destroy();
	}

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

void AArenaCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// [client] as soon as PlayerState is assigned, set team colors of this pawn for local player
	if (PlayerState != NULL)
	{
		UpdateTeamColorsAllMIDs();
	}
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

void AArenaCharacter::OnRep_PrimaryWeapon()
{
	if (PrimaryWeapon)
	{
		PrimaryWeapon->SetPrimary(true);
		PrimaryWeapon->SetOwningPawn(this);
		PrimaryWeapon->FinishUnEquip();
	}
}

void AArenaCharacter::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon)
	{
		SecondaryWeapon->SetOwningPawn(this);
		SecondaryWeapon->FinishUnEquip();
	}
}

void AArenaCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AArenaCharacter, LastTakeHitInfo, COND_Custom);

	DOREPLIFETIME(AArenaCharacter, Name);
	DOREPLIFETIME(AArenaCharacter, Busy);
	DOREPLIFETIME(AArenaCharacter, Peaking);
	DOREPLIFETIME(AArenaCharacter, ActionQueue);
	DOREPLIFETIME(AArenaCharacter, CurrentWeapon);
	DOREPLIFETIME(AArenaCharacter, PrimaryWeapon);
	DOREPLIFETIME(AArenaCharacter, SecondaryWeapon);

	DOREPLIFETIME(AArenaCharacter, HeadUtility);
	DOREPLIFETIME(AArenaCharacter, UpperBackUtility);
	DOREPLIFETIME(AArenaCharacter, LowerBackUtility);
	DOREPLIFETIME(AArenaCharacter, LeftWristUtility);
	DOREPLIFETIME(AArenaCharacter, RightWristUtility);
	DOREPLIFETIME(AArenaCharacter, LeftWaistUtility);
	DOREPLIFETIME(AArenaCharacter, RightWaistUtility);
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

bool AArenaCharacter::ServerJump_Validate(class AArenaCharacter* client)
{
	return true;
}
void AArenaCharacter::ServerJump_Implementation(class AArenaCharacter* client)
{
	//client->PlayerConfig.Stamina -= JumpCost;
}

bool AArenaCharacter::ServerVault_Validate()
{
	return true;
}
void AArenaCharacter::ServerVault_Implementation()
{
	StartVault();
}

bool AArenaCharacter::ServerClimb_Validate()
{
	return true;
}
void AArenaCharacter::ServerClimb_Implementation()
{
	StartClimb();
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
	StopTargeting();
}

bool AArenaCharacter::ServerStartPeaking_Validate()
{
	return true;
}
void AArenaCharacter::ServerStartPeaking_Implementation()
{
	StartPeaking();
}

bool AArenaCharacter::ServerStopPeaking_Validate()
{
	return true;
}
void AArenaCharacter::ServerStopPeaking_Implementation()
{
	if (Busy)
	{
		GetWorldTimerManager().SetTimer(this, &AArenaCharacter::StopPeaking, ActionQueue * 0.4f, false);
	}
	else
	{
		StopPeaking();
	}
}

bool AArenaCharacter::ServerSetName_Validate(const FString& NewName)
{
	return true;
}
void AArenaCharacter::ServerSetName_Implementation(const FString& NewName)
{
	SetName(NewName);
}

bool AArenaCharacter::ServerSpawnEquipment_Validate(TSubclassOf<class AArenaWeapon> MainWeapon, TSubclassOf<class AArenaWeapon> OffWeapon, TSubclassOf<class AArenaUtility> UpperBack)
{
	return true;
}
void AArenaCharacter::ServerSpawnEquipment_Implementation(TSubclassOf<class AArenaWeapon> MainWeapon, TSubclassOf<class AArenaWeapon> OffWeapon, TSubclassOf<class AArenaUtility>  UpperBack)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	if (MainWeapon)
	{
		PrimaryWeapon = GetWorld()->SpawnActor<AArenaWeapon>(MainWeapon, SpawnInfo);
		PrimaryWeapon->SetPrimary(true);
	}
	if (OffWeapon)
	{
		SecondaryWeapon = GetWorld()->SpawnActor<AArenaWeapon>(OffWeapon, SpawnInfo);
	}
	if (UpperBack)
	{
		UpperBackUtility = GetWorld()->SpawnActor<AArenaUtility>(UpperBack, SpawnInfo);
	}

	//HeadUtility = GetWorld()->SpawnActor<AArenaUtility>(CharacterEquipment->GetHeadUtilityBP(), SpawnInfo);

	//UpperBackUtility = GetWorld()->SpawnActor<AArenaUtility>(CharacterEquipment->GetUpperBackUtilityBP(), SpawnInfo);
	//LowerBackUtility = GetWorld()->SpawnActor<AArenaUtility>(CharacterEquipment->GetLowerBackUtilityBP(), SpawnInfo);

	//LeftWristUtility = GetWorld()->SpawnActor<AArenaUtility>(CharacterEquipment->GetLeftWristUtilityBP(), SpawnInfo);
	//RightWristUtility = GetWorld()->SpawnActor<AArenaUtility>(CharacterEquipment->GetRightWristUtilityBP(), SpawnInfo);

	//LeftWaistUtility = GetWorld()->SpawnActor<AArenaUtility>(CharacterEquipment->GetLeftWaistUtilityBP(), SpawnInfo);
	//RightWaistUtility = GetWorld()->SpawnActor<AArenaUtility>(CharacterEquipment->GetRightWaistUtilityBP(), SpawnInfo);

	InitializeWeapons(PrimaryWeapon, SecondaryWeapon, UpperBackUtility);
}

bool AArenaCharacter::ServerToggleCrouch_Validate()
{
	return true;
}
void AArenaCharacter::ServerToggleCrouch_Implementation()
{
	ToggleCrouch();
}

bool AArenaCharacter::ServerToggleCover_Validate()
{
	return true;
}
void AArenaCharacter::ServerToggleCover_Implementation()
{
	ToggleCover();
}

bool AArenaCharacter::ServerExitCover_Validate()
{
	return true;
}
void AArenaCharacter::ServerExitCover_Implementation()
{
	ExitCover();
}

bool AArenaCharacter::ServerRunning_Validate(bool IsRunning)
{
	return true;
}
void AArenaCharacter::ServerRunning_Implementation(bool IsRunning)
{
	Running(IsRunning);
}

bool AArenaCharacter::ServerEnterCombat_Validate()
{
	return true;
}
void AArenaCharacter::ServerEnterCombat_Implementation()
{
	ToggleCombat();
}

bool AArenaCharacter::ServerSwapWeapon_Validate()
{
	return true;
}
void AArenaCharacter::ServerSwapWeapon_Implementation()
{
	SwapWeapon();
}

bool AArenaCharacter::ServerInitializeWeapons_Validate(AArenaWeapon* mainWeapon, AArenaWeapon* offWeapon, AArenaUtility* UpperBack)
{
	return true;
}
void AArenaCharacter::ServerInitializeWeapons_Implementation(AArenaWeapon* mainWeapon, AArenaWeapon* offWeapon, AArenaUtility* UpperBack)
{
	InitializeWeapons(mainWeapon, offWeapon, UpperBack);
}

bool AArenaCharacter::ServerSetPrimaryWeapon_Validate(TSubclassOf<class AArenaWeapon> Weapon)
{
	return true;
}
void AArenaCharacter::ServerSetPrimaryWeapon_Implementation(TSubclassOf<class AArenaWeapon> Weapon)
{
	HandlePrimaryWeapon(Weapon);
}

bool AArenaCharacter::ServerSetSecondaryWeapon_Validate(TSubclassOf<class AArenaWeapon> Weapon)
{
	return true;
}
void AArenaCharacter::ServerSetSecondaryWeapon_Implementation(TSubclassOf<class AArenaWeapon> Weapon)
{
	HandleSecondaryWeapon(Weapon);
}

bool AArenaCharacter::ServerSetUpperBackUtility_Validate(TSubclassOf<class AArenaUtility> Utility)
{
	return true;
}
void AArenaCharacter::ServerSetUpperBackUtility_Implementation(TSubclassOf<class AArenaUtility> Utility)
{
	HandleUpperBackUtility(Utility);
}