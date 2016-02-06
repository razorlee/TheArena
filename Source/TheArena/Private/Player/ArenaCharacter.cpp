// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "TheArena.h"

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

	Http = &FHttpModule::Get();
	TargetHost = FString::Printf(TEXT("http://www.appspot.com"));
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
	CharacterState->Reset(); //test without this

	LoadPersistence();

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
	if (CharacterState->GetPlayerState() == EPlayerState::Crouching)
	{
		Crouch();
	}
}

void AArenaCharacter::SaveCharacter()
{
	// this will be a function that sends an updated JSON object to server
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

	SaveGameInstance->ChestArmor = CharacterEquipment->GetChestArmorBP();
	SaveGameInstance->HandArmor = CharacterEquipment->GetHandsArmorBP();
	SaveGameInstance->HeadArmor = CharacterEquipment->GetHeadArmorBP();
	SaveGameInstance->FeetArmor = CharacterEquipment->GetFeetArmorBP();
	SaveGameInstance->LegArmor = CharacterEquipment->GetLegsArmorBP();
	SaveGameInstance->ShoulderArmor = CharacterEquipment->GetShoulderArmorBP();

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, Name, SaveGameInstance->UserIndex);
}

void AArenaCharacter::LoadPersistence()
{
	AArenaPlayerState* MyPlayerState = Cast<AArenaPlayerState>(PlayerState);
	if (PlayerState)
	{
		Name = MyPlayerState->PlayerName;
	}

	TSharedRef < IHttpRequest > Request = Http->CreateRequest();
	Request->SetVerb("POST");
	Request->SetURL(TargetHost);
	Request->SetContentAsString("Poop");
	Request->SetHeader("Content-Type", "application/x-www-form-urlencoded");
	Request->OnProcessRequestComplete().BindUObject(this, &AArenaCharacter::OnResponseReceived);
	Request->ProcessRequest();

	if (IsLocallyControlled())
	{
		if (Role == ROLE_Authority)
		{
			ApplyArmorStats();
		}
		else
		{
			ServerApplyArmorStats();
		}
	}
}

void AArenaCharacter::SpawnEquipment()
{
	if (Role == ROLE_Authority)
	{
		//FString ResponseBody = Response->GetContentAsString();

		//TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
		//TSharedRef< TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(ResponseBody);

		//if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
		//{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.bNoCollisionFail = true;

		////////////////////////////////////////////////// WEAPONS //////////////////////////////////////////////////

		//JsonObject->GetStringField(TEXT("Main Weapon"));
		PrimaryWeapon = GetWorld()->SpawnActor<AArenaWeapon>(CharacterEquipment->GetPrimaryWeaponBP(), SpawnInfo);
		PrimaryWeapon->SetPrimary(true);
		//JsonObject->GetStringField(TEXT("Secondary Weapon"));
		SecondaryWeapon = GetWorld()->SpawnActor<AArenaWeapon>(CharacterEquipment->GetSecondaryWeaponBP(), SpawnInfo);

		///////////////////////////////////////////////// UTILITIES /////////////////////////////////////////////////

		HeadUtility = GetWorld()->SpawnActor<AArenaUtility>(CharacterEquipment->GetHeadUtilityBP(), SpawnInfo);
		UpperBackUtility = GetWorld()->SpawnActor<AArenaUtility>(CharacterEquipment->GetUpperBackUtilityBP(), SpawnInfo);
		LowerBackUtility = GetWorld()->SpawnActor<AArenaUtility>(CharacterEquipment->GetLowerBackUtilityBP(), SpawnInfo);
		LeftWaistUtility = GetWorld()->SpawnActor<AArenaUtility>(CharacterEquipment->GetLeftWaistUtilityBP(), SpawnInfo);
		RightWaistUtility = GetWorld()->SpawnActor<AArenaUtility>(CharacterEquipment->GetRightWaistUtilityBP(), SpawnInfo);
		LeftWristUtility = GetWorld()->SpawnActor<AArenaUtility>(CharacterEquipment->GetLeftWristUtilityBP(), SpawnInfo);
		RightWristUtility = GetWorld()->SpawnActor<AArenaUtility>(CharacterEquipment->GetRightWristUtilityBP(), SpawnInfo);

		////////////////////////////////////////////////////// ARMOR //////////////////////////////////////////////////////

		ChestArmor = GetWorld()->SpawnActor<AArenaArmor>(CharacterEquipment->GetChestArmorBP(), SpawnInfo);
		HandArmor = GetWorld()->SpawnActor<AArenaArmor>(CharacterEquipment->GetHandsArmorBP(), SpawnInfo);
		HeadArmor = GetWorld()->SpawnActor<AArenaArmor>(CharacterEquipment->GetHeadArmorBP(), SpawnInfo);
		FeetArmor = GetWorld()->SpawnActor<AArenaArmor>(CharacterEquipment->GetFeetArmorBP(), SpawnInfo);
		LegArmor = GetWorld()->SpawnActor<AArenaArmor>(CharacterEquipment->GetLegsArmorBP(), SpawnInfo);
		ShoulderArmor = GetWorld()->SpawnActor<AArenaArmor>(CharacterEquipment->GetShoulderArmorBP(), SpawnInfo);

		InitializeWeapons();
	}
	else
	{
		ServerSpawnEquipment();
	}
}

void AArenaCharacter::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (Response.IsValid())
		{
			SpawnEquipment();
		}
		else
		{

		}
	}
	else
	{

	}
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

	AArenaPlayerState* MyPlayerState = Cast<AArenaPlayerState>(PlayerState); //test without this
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

	InputComponent->BindAction("UpperBack", IE_Pressed, this, &AArenaCharacter::OnActivateBack);
	InputComponent->BindAction("UpperBack", IE_Released, this, &AArenaCharacter::OnDeactivateBack);

	InputComponent->BindAction("LeftWaist", IE_Pressed, this, &AArenaCharacter::OnActivateLeftWaist);
	InputComponent->BindAction("LeftWaist", IE_Released, this, &AArenaCharacter::OnDeactivateLeftWaist);

	InputComponent->BindAction("RightWaist", IE_Pressed, this, &AArenaCharacter::OnActivateRightWaist);
	InputComponent->BindAction("RightWaist", IE_Released, this, &AArenaCharacter::OnDeactivateRightWaist);

	InputComponent->BindAction("LeftWrist", IE_Pressed, this, &AArenaCharacter::OnActivateLeftWrist);
	InputComponent->BindAction("LeftWrist", IE_Released, this, &AArenaCharacter::OnDeactivateLeftWrist);

	InputComponent->BindAction("RightWrist", IE_Pressed, this, &AArenaCharacter::OnActivateRightWrist);
	InputComponent->BindAction("RightWrist", IE_Released, this, &AArenaCharacter::OnDeactivateRightWrist);
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
		Jump();
		if (Role == ROLE_Authority)
		{
			CharacterAttributes->SetCurrentStamina(CharacterAttributes->GetCurrentStamina() - CharacterMovementComponent->CostConfig.JumpCost);
		}
		else
		{
			ServerJump(this);
		}
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
				GetWorldTimerManager().SetTimer(TimerHandle_StopPeaking, this, &AArenaCharacter::StopPeaking, ActionQueue * 0.3f, false);
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
void AArenaCharacter::OnActivateLeftWaist()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Waist(LeftWaistUtility, this, MyPC))
	{
		LeftWaistUtility->Activate();
	}
}
void AArenaCharacter::OnDeactivateLeftWaist()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (LeftWaistUtility)
	{
		LeftWaistUtility->Deactivate();
	}
}
void AArenaCharacter::OnActivateRightWaist()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Waist(RightWaistUtility, this, MyPC))
	{
		RightWaistUtility->Activate();
	}
}
void AArenaCharacter::OnDeactivateRightWaist()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (RightWaistUtility)
	{
		RightWaistUtility->Deactivate();
	}
}
void AArenaCharacter::OnActivateLeftWrist()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Wrist(LeftWristUtility, this, MyPC))
	{
		if (LeftWristUtility->GetActivationType() == EActivationType::Activate)
		{
			OnStartTargeting();
		}
		else
		{
			LeftWristUtility->Activate();
		}
	}
}
void AArenaCharacter::OnDeactivateLeftWrist()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (LeftWristUtility)
	{
		if (LeftWristUtility->GetActivationType() == EActivationType::Activate && ArenaCharacterCan::Wrist(LeftWristUtility, this, MyPC))
		{
			LeftWristUtility->Activate();
			OnStopTargeting();
			LeftWristUtility->Deactivate();
		}
		else
		{
			LeftWristUtility->Deactivate();
		}
	}
}
void AArenaCharacter::OnActivateRightWrist()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (ArenaCharacterCan::Wrist(RightWristUtility, this, MyPC))
	{
		if (RightWristUtility->GetActivationType() == EActivationType::Activate)
		{
			OnStartTargeting();
		}
		else
		{
			RightWristUtility->Activate();
		}
	}
}
void AArenaCharacter::OnDeactivateRightWrist()
{
	AArenaPlayerController* MyPC = Cast<AArenaPlayerController>(Controller);
	if (RightWristUtility)
	{
		if (RightWristUtility->GetActivationType() == EActivationType::Activate && ArenaCharacterCan::Wrist(RightWristUtility, this, MyPC))
		{
			RightWristUtility->Activate();
			OnStopTargeting();
			RightWristUtility->Deactivate();
		}
		else
		{
			RightWristUtility->Deactivate();
		}
	}
}

////////////////////////////////////////// Action Functions //////////////////////////////////////////

void AArenaCharacter::InitializeWeapons()
{
	if (Role == ROLE_Authority)
	{
		PrimaryWeapon->SetOwningPawn(this);
		PrimaryWeapon->FinishUnEquip();

		SecondaryWeapon->SetOwningPawn(this);;
		SecondaryWeapon->FinishUnEquip();

	//////////////////////////////////////////////////////////////////////////////////////////////////

		HeadUtility->SetMyPawn(this);

		UpperBackUtility->SetMyPawn(this);
		UpperBackUtility->Equip();

		LowerBackUtility->SetMyPawn(this);

		LeftWristUtility->SetMyPawn(this);

		RightWristUtility->SetMyPawn(this);

		LeftWaistUtility->SetMyPawn(this);
		LeftWaistUtility->Equip();

		RightWaistUtility->SetMyPawn(this);
		RightWaistUtility->Equip();

	//////////////////////////////////////////////////////////////////////////////////////////////////

		ChestArmor->SetMyPawn(this);
		ChestArmor->Equip();

		HandArmor->SetMyPawn(this);
		HandArmor->Equip();

		HeadArmor->SetMyPawn(this);
		HeadArmor->Equip();

		FeetArmor->SetMyPawn(this);
		FeetArmor->Equip();

		LegArmor->SetMyPawn(this);
		LegArmor->Equip();

		ShoulderArmor->SetMyPawn(this);
		ShoulderArmor->Equip();
	}
	else
	{
		ServerInitializeWeapons();
	}
}

void AArenaCharacter::ApplyArmorStats()
{
	this;
	CharacterAttributes->SetProtection(0.0);
	CharacterAttributes->SetSpeed(0.0);

	if (ChestArmor)
	{
		CharacterAttributes->SetProtection(CharacterAttributes->GetProtection() + ChestArmor->GetProtection());
		CharacterAttributes->SetSpeed(CharacterAttributes->GetSpeed() + ChestArmor->GetMotility());
	}
	if (HandArmor)
	{
		CharacterAttributes->SetProtection(CharacterAttributes->GetProtection() + HandArmor->GetProtection());
		CharacterAttributes->SetSpeed(CharacterAttributes->GetSpeed() + HandArmor->GetMotility());
	}
	if (HeadArmor)
	{
		CharacterAttributes->SetProtection(CharacterAttributes->GetProtection() + HeadArmor->GetProtection());
		CharacterAttributes->SetSpeed(CharacterAttributes->GetSpeed() + HeadArmor->GetMotility());
	}
	if (FeetArmor)
	{
		CharacterAttributes->SetProtection(CharacterAttributes->GetProtection() + FeetArmor->GetProtection());
		CharacterAttributes->SetSpeed(CharacterAttributes->GetSpeed() + FeetArmor->GetMotility());
	}
	if (LegArmor)
	{
		CharacterAttributes->SetProtection(CharacterAttributes->GetProtection() + LegArmor->GetProtection());
		CharacterAttributes->SetSpeed(CharacterAttributes->GetSpeed() + LegArmor->GetMotility());
	}
	if (ShoulderArmor)
	{
		CharacterAttributes->SetProtection(CharacterAttributes->GetProtection() + ShoulderArmor->GetProtection());
		CharacterAttributes->SetSpeed(CharacterAttributes->GetSpeed() + ShoulderArmor->GetMotility());
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
		UGameplayStatics::SpawnSoundAttached(TargetingSound, GetRootComponent());
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
	else if (CharacterState->GetCoverState() == ECoverState::LowMiddle)
	{
		//Busy = true;
		CurrentWeapon->GetWeaponState()->SetCoverTargeting(true);
		//CharacterMovementComponent->SetLocation(GetActorLocation());
	}
	else
	{
		return;
	}
	GetWorldTimerManager().SetTimer(TimerHandle_Busy, this, &AArenaCharacter::ToggleBusy, ActionQueue * 0.40f, false);
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
	else if (CharacterState->GetCoverState() == ECoverState::LowMiddle)
	{
		CurrentWeapon->GetWeaponState()->SetCoverTargeting(false);
	}
	else
	{
		return;
	}
	GetWorldTimerManager().SetTimer(TimerHandle_SetLocation, this, &AArenaCharacter::SetLocation, ActionQueue * 0.4f, false);
	GetWorldTimerManager().SetTimer(TimerHandle_Busy, this, &AArenaCharacter::ToggleBusy, ActionQueue * 0.4f, false);
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
	CharacterAttributes->SetCurrentStamina(CharacterAttributes->GetCurrentStamina() - CharacterMovementComponent->CostConfig.VaultCost);

	float AnimDuration = PlayAnimMontage(CharacterMovementComponent->GetVaultAnimation());
	if (AnimDuration <= 0.0f)
	{
		AnimDuration = 0.3f;
	}

	GetWorldTimerManager().SetTimer(TimerHandle_Vault, this, &AArenaCharacter::StopVault, AnimDuration * 0.9f, false);
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
	CharacterAttributes->SetCurrentStamina(CharacterAttributes->GetCurrentStamina() - CharacterMovementComponent->CostConfig.ClimbCost);

	float AnimDuration = PlayAnimMontage(CharacterMovementComponent->GetClimbAnimation());
	if (AnimDuration <= 0.0f)
	{
		AnimDuration = 0.3f;
	}

	GetWorldTimerManager().SetTimer(TimerHandle_Vault, this, &AArenaCharacter::StopClimb, AnimDuration * 0.9f, false);
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

////////////////////////////////////////// Damage & Death //////////////////////////////////////////

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
	CharacterEquipment->SetPrimaryWeaponBP(Weapon);
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

	if (IsLocallyControlled() && Weapon)
	{
		SaveCharacter();
	}
}

AArenaWeapon* AArenaCharacter::GetSecondaryWeapon()
{
	return SecondaryWeapon;
}
void AArenaCharacter::SetSecondaryWeapon(TSubclassOf<class AArenaWeapon> Weapon)
{
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
	CharacterEquipment->SetSecondaryWeaponBP(Weapon);
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

	if (IsLocallyControlled() && Weapon)
	{
		SaveCharacter();
	}
}

////////////////////////////////////////// Damage & Death //////////////////////////////////////////

class AArenaUtility* AArenaCharacter::GetHeadUtility()
{
	return HeadUtility;
}
void AArenaCharacter::SetHeadUtility(TSubclassOf<class AArenaUtility> Utility)
{
	if (Role == ROLE_Authority)
	{
		HandleHeadUtility(Utility);
	}
	else
	{
		ServerSetHeadUtility(Utility);
	}
}
void AArenaCharacter::HandleHeadUtility(TSubclassOf<class AArenaUtility> Utility)
{
	CharacterEquipment->SetHeadUtilityBP(Utility);
	AArenaUtility* TBD = HeadUtility;
	if (TBD)
	{
		TBD->UnEquip();
		TBD->Destroy();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	HeadUtility = GetWorld()->SpawnActor<AArenaUtility>(Utility, SpawnInfo);
	HeadUtility->SetMyPawn(this);

	if (IsLocallyControlled() && Utility)
	{
		SaveCharacter();
	}
}

class AArenaUtility* AArenaCharacter::GetUpperBackUtility()
{
	return UpperBackUtility;
}
void AArenaCharacter::SetUpperBackUtility(TSubclassOf<class AArenaUtility> Utility)
{
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
	CharacterEquipment->SetUpperBackUtilityBP(Utility);
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

	if (IsLocallyControlled() && Utility)
	{
		SaveCharacter();
	}
}

class AArenaUtility* AArenaCharacter::GetLowerBackUtility()
{
	return LowerBackUtility;
}
void AArenaCharacter::SetLowerBackUtility(TSubclassOf<class AArenaUtility> Utility)
{
	if (Role == ROLE_Authority)
	{
		HandleLowerBackUtility(Utility);
	}
	else
	{
		ServerSetLowerBackUtility(Utility);
	}
}
void AArenaCharacter::HandleLowerBackUtility(TSubclassOf<class AArenaUtility> Utility)
{
	CharacterEquipment->SetLowerBackUtilityBP(Utility);
	AArenaUtility* TBD = LowerBackUtility;
	if (TBD)
	{
		TBD->UnEquip();
		TBD->Destroy();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	LowerBackUtility = GetWorld()->SpawnActor<AArenaUtility>(Utility, SpawnInfo);
	LowerBackUtility->SetMyPawn(this);

	if (IsLocallyControlled() && Utility)
	{
		SaveCharacter();
	}
}

class AArenaUtility* AArenaCharacter::GetLeftWristUtility()
{
	return LeftWristUtility;
}
void AArenaCharacter::SetLeftWristUtility(TSubclassOf<class AArenaUtility> Utility)
{
	if (Role == ROLE_Authority)
	{
		HandleLeftWristUtility(Utility);
	}
	else
	{
		ServerSetLeftWristUtility(Utility);
	}
}
void AArenaCharacter::HandleLeftWristUtility(TSubclassOf<class AArenaUtility> Utility)
{
	CharacterEquipment->SetLeftWristUtilityBP(Utility);
	AArenaUtility* TBD = LeftWristUtility;
	if (TBD)
	{
		TBD->UnEquip();
		TBD->Destroy();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	LeftWristUtility = GetWorld()->SpawnActor<AArenaUtility>(Utility, SpawnInfo);
	LeftWristUtility->SetMyPawn(this);

	if (IsLocallyControlled() && Utility)
	{
		SaveCharacter();
	}
}

class AArenaUtility* AArenaCharacter::GetRightWristUtility()
{
	return RightWristUtility;
}
void AArenaCharacter::SetRightWristUtility(TSubclassOf<class AArenaUtility> Utility)
{
	if (Role == ROLE_Authority)
	{
		HandleRightWristUtility(Utility);
	}
	else
	{
		ServerSetRightWristUtility(Utility);
	}
}
void AArenaCharacter::HandleRightWristUtility(TSubclassOf<class AArenaUtility> Utility)
{
	CharacterEquipment->SetRightWristUtilityBP(Utility);
	AArenaUtility* TBD = RightWristUtility;
	if (TBD)
	{
		TBD->UnEquip();
		TBD->Destroy();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	RightWristUtility = GetWorld()->SpawnActor<AArenaUtility>(Utility, SpawnInfo);
	RightWristUtility->SetMyPawn(this);

	if (IsLocallyControlled() && Utility)
	{
		SaveCharacter();
	}
}

class AArenaUtility* AArenaCharacter::GetLeftWaistUtility()
{
	return LeftWaistUtility;
}
void AArenaCharacter::SetLeftWaistUtility(TSubclassOf<class AArenaUtility> Utility)
{
	if (Role == ROLE_Authority)
	{
		HandleLeftWaistUtility(Utility);
	}
	else
	{
		ServerSetLeftWaistUtility(Utility);
	}
}
void AArenaCharacter::HandleLeftWaistUtility(TSubclassOf<class AArenaUtility> Utility)
{
	CharacterEquipment->SetLeftWaistUtilityBP(Utility);
	AArenaUtility* TBD = LeftWaistUtility;
	if (TBD)
	{
		TBD->UnEquip();
		TBD->Destroy();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	LeftWaistUtility = GetWorld()->SpawnActor<AArenaUtility>(Utility, SpawnInfo);
	LeftWaistUtility->SetMyPawn(this);

	if (IsLocallyControlled() && Utility)
	{
		SaveCharacter();
	}
}

class AArenaUtility* AArenaCharacter::GetRightWaistUtility()
{
	return RightWaistUtility;
}
void AArenaCharacter::SetRightWaistUtility(TSubclassOf<class AArenaUtility> Utility)
{
	if (Role == ROLE_Authority)
	{
		HandleRightWaistUtility(Utility);
	}
	else
	{
		ServerSetRightWaistUtility(Utility);
	}
}
void AArenaCharacter::HandleRightWaistUtility(TSubclassOf<class AArenaUtility> Utility)
{
	CharacterEquipment->SetRightWaistUtilityBP(Utility);
	AArenaUtility* TBD = RightWaistUtility;
	if (TBD)
	{
		TBD->UnEquip();
		TBD->Destroy();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	RightWaistUtility = GetWorld()->SpawnActor<AArenaUtility>(Utility, SpawnInfo);
	RightWaistUtility->SetMyPawn(this);

	if (IsLocallyControlled() && Utility)
	{
		SaveCharacter();
	}
}

////////////////////////////////////////// Damage & Death //////////////////////////////////////////

class AArenaArmor* AArenaCharacter::GetHeadArmor()
{
	return HeadArmor;
}
void AArenaCharacter::SetHeadArmor(TSubclassOf<class AArenaArmor> Armor)
{
	if (Role == ROLE_Authority)
	{
		HandleHeadArmor(Armor);
	}
	else
	{
		ServerSetHeadArmor(Armor);
	}
}
void AArenaCharacter::HandleHeadArmor(TSubclassOf<class AArenaArmor> Armor)
{
	CharacterEquipment->SetHeadArmorBP(Armor);
	AArenaArmor* TBD = HeadArmor;
	if (TBD)
	{
		TBD->UnEquip();
		TBD->Destroy();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	HeadArmor = GetWorld()->SpawnActor<AArenaArmor>(Armor, SpawnInfo);
	HeadArmor->SetMyPawn(this);
	ApplyArmorStats();

	if (IsLocallyControlled() && Armor)
	{
		SaveCharacter();
	}
}

class AArenaArmor* AArenaCharacter::GetChestArmor()
{
	return ChestArmor;
}
void AArenaCharacter::SetChestArmor(TSubclassOf<class AArenaArmor> Armor)
{
	if (Role == ROLE_Authority)
	{
		HandleChestArmor(Armor);
	}
	else
	{
		ServerSetChestArmor(Armor);
	}
}
void AArenaCharacter::HandleChestArmor(TSubclassOf<class AArenaArmor> Armor)
{
	CharacterEquipment->SetChestArmorBP(Armor);
	AArenaArmor* TBD = ChestArmor;
	if (TBD)
	{
		TBD->UnEquip();
		TBD->Destroy();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	ChestArmor = GetWorld()->SpawnActor<AArenaArmor>(Armor, SpawnInfo);
	ChestArmor->SetMyPawn(this);
	ApplyArmorStats();

	if (IsLocallyControlled() && Armor)
	{
		SaveCharacter();
	}
}

class AArenaArmor* AArenaCharacter::GetShoulderArmor()
{
	return ShoulderArmor;
}
void AArenaCharacter::SetShoulderArmor(TSubclassOf<class AArenaArmor> Armor)
{
	if (Role == ROLE_Authority)
	{
		HandleShoulderArmor(Armor);
	}
	else
	{
		ServerSetShoulderArmor(Armor);
	}
}
void AArenaCharacter::HandleShoulderArmor(TSubclassOf<class AArenaArmor> Armor)
{
	CharacterEquipment->SetShoulderArmorBP(Armor);
	AArenaArmor* TBD = ShoulderArmor;
	if (TBD)
	{
		TBD->UnEquip();
		TBD->Destroy();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	ShoulderArmor = GetWorld()->SpawnActor<AArenaArmor>(Armor, SpawnInfo);
	ShoulderArmor->SetMyPawn(this);
	ApplyArmorStats();

	if (IsLocallyControlled() && Armor)
	{
		SaveCharacter();
	}
}

class AArenaArmor* AArenaCharacter::GetHandArmor()
{
	return HandArmor;
}
void AArenaCharacter::SetHandArmor(TSubclassOf<class AArenaArmor> Armor)
{
	if (Role == ROLE_Authority)
	{
		HandleHandArmor(Armor);
	}
	else
	{
		ServerSetHandArmor(Armor);
	}
}
void AArenaCharacter::HandleHandArmor(TSubclassOf<class AArenaArmor> Armor)
{
	CharacterEquipment->SetHandsArmorBP(Armor);
	AArenaArmor* TBD = HandArmor;
	if (TBD)
	{
		TBD->UnEquip();
		TBD->Destroy();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	HandArmor = GetWorld()->SpawnActor<AArenaArmor>(Armor, SpawnInfo);
	HandArmor->SetMyPawn(this);
	ApplyArmorStats();

	if (IsLocallyControlled() && Armor)
	{
		SaveCharacter();
	}
}

class AArenaArmor* AArenaCharacter::GetLegArmor()
{
	return LegArmor;
}
void AArenaCharacter::SetLegArmor(TSubclassOf<class AArenaArmor> Armor)
{
	if (Role == ROLE_Authority)
	{
		HandleLegArmor(Armor);
	}
	else
	{
		ServerSetLegArmor(Armor);
	}
}
void AArenaCharacter::HandleLegArmor(TSubclassOf<class AArenaArmor> Armor)
{
	CharacterEquipment->SetLegsArmorBP(Armor);
	AArenaArmor* TBD = LegArmor;
	if (TBD)
	{
		TBD->UnEquip();
		TBD->Destroy();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	LegArmor = GetWorld()->SpawnActor<AArenaArmor>(Armor, SpawnInfo);
	LegArmor->SetMyPawn(this);
	ApplyArmorStats();

	if (IsLocallyControlled() && Armor)
	{
		SaveCharacter();
	}
}

class AArenaArmor* AArenaCharacter::GetFeetArmor()
{
	return FeetArmor;
}
void AArenaCharacter::SetFeetArmor(TSubclassOf<class AArenaArmor> Armor)
{
	if (Role == ROLE_Authority)
	{
		HandleFeetArmor(Armor);
	}
	else
	{
		ServerSetFeetArmor(Armor);
	}
}
void AArenaCharacter::HandleFeetArmor(TSubclassOf<class AArenaArmor> Armor)
{
	CharacterEquipment->SetFeetArmorBP(Armor);
	AArenaArmor* TBD = FeetArmor;
	if (TBD)
	{
		TBD->UnEquip();
		TBD->Destroy();
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	FeetArmor = GetWorld()->SpawnActor<AArenaArmor>(Armor, SpawnInfo);
	FeetArmor->SetMyPawn(this);
	ApplyArmorStats();

	if (IsLocallyControlled() && Armor)
	{
		SaveCharacter();
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

	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage != 0.f)
	{
		if (CharacterAttributes->GetCurrentShields() > 0 && ActualDamage > 0)
		{
			float temp = ActualDamage - CharacterAttributes->GetCurrentShields();
			CharacterAttributes->SetCurrentShields(CharacterAttributes->GetCurrentShields() - ActualDamage);
			if (temp > 0)
			{
				ActualDamage = temp;
			}
		}
		if (CharacterAttributes->GetCurrentShields() <= 0 || ActualDamage < 0)
		{
			ActualDamage = ActualDamage * (1 - (CharacterAttributes->GetProtection() / 2000.0f)); //2000 is 100% protection
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

	AArenaPlayerController* PC = Cast<AArenaPlayerController>(Controller);
	PC->SetMenu(false);
	PC->SetFriendsList(false);
	PC->SetSettings(false);
	PC->SetInventory(false);

	CharacterAttributes->SetCurrentHealth(FMath::Min(0.0f, CharacterAttributes->GetCurrentHealth()));

	// if this is an environmental death then refer to the previous killer so that they receive credit (knocked into lava pits, etc)
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	AController* const KilledPlayer = (Controller != NULL) ? Controller : Cast<AController>(GetOwner());
	

	NetUpdateFrequency = GetDefault<AArenaCharacter>()->NetUpdateFrequency;
	GetCharacterMovement()->ForceReplicationUpdate();

	OnDeath(KillingDamage, DamageEvent, Killer ? Killer->GetPawn() : NULL, DamageCauser);
	GetWorld()->GetAuthGameMode<ATheArenaGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);
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

	AArenaPlayerController* PC = Cast<AArenaPlayerController>(Controller);
	if (Role == ROLE_Authority)
	{
		ReplicateHit(KillingDamage, DamageEvent, PawnInstigator, DamageCauser, true);

		// play the force feedback effect on the client player controller
		if (PC && DamageEvent.DamageTypeClass)
		{
			UArenaDamageType *DamageType = Cast<UArenaDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
			if (DamageType && DamageType->KilledForceFeedback)
			{
				PC->ClientPlayForceFeedback(DamageType->KilledForceFeedback, false, "Damage");
			}
		}
		PC->EnterSpectatorMode();
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
		SetLifeSpan(900.0f);
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

	Utility = LeftWaistUtility;
	if (Utility)
	{
		Utility->UnEquip();
		Utility->Destroy();
	}

	Utility = RightWaistUtility;
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

	//DOREPLIFETIME(AArenaCharacter, CharacterEquipment);
	//DOREPLIFETIME(AArenaCharacter, CharacterAttributes);

	DOREPLIFETIME(AArenaCharacter, HeadUtility);
	DOREPLIFETIME(AArenaCharacter, UpperBackUtility);
	DOREPLIFETIME(AArenaCharacter, LowerBackUtility);
	DOREPLIFETIME(AArenaCharacter, LeftWristUtility);
	DOREPLIFETIME(AArenaCharacter, RightWristUtility);
	DOREPLIFETIME(AArenaCharacter, LeftWaistUtility);
	DOREPLIFETIME(AArenaCharacter, RightWaistUtility);

	DOREPLIFETIME(AArenaCharacter, HeadArmor);
	DOREPLIFETIME(AArenaCharacter, ShoulderArmor);
	DOREPLIFETIME(AArenaCharacter, ChestArmor);
	DOREPLIFETIME(AArenaCharacter, HandArmor);
	DOREPLIFETIME(AArenaCharacter, LegArmor);
	DOREPLIFETIME(AArenaCharacter, FeetArmor);
}

////////////////////////////////////////////// Server //////////////////////////////////////////////

bool AArenaCharacter::ServerApplyArmorStats_Validate()
{
	return true;
}
void AArenaCharacter::ServerApplyArmorStats_Implementation()
{
	ApplyArmorStats();
}

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
	CharacterAttributes->SetCurrentStamina(CharacterAttributes->GetCurrentStamina() - CharacterMovementComponent->CostConfig.JumpCost);
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
		GetWorldTimerManager().SetTimer(TimerHandle_Vault, this, &AArenaCharacter::StopPeaking, ActionQueue * 0.4f, false);
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

bool AArenaCharacter::ServerSpawnEquipment_Validate()
{
	return true;
}
void AArenaCharacter::ServerSpawnEquipment_Implementation()
{
	
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

bool AArenaCharacter::ServerInitializeWeapons_Validate()
{
	return true;
}
void AArenaCharacter::ServerInitializeWeapons_Implementation()
{
	InitializeWeapons();
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

bool AArenaCharacter::ServerSetHeadUtility_Validate(TSubclassOf<class AArenaUtility> Utility)
{
	return true;
}
void AArenaCharacter::ServerSetHeadUtility_Implementation(TSubclassOf<class AArenaUtility> Utility)
{
	HandleHeadUtility(Utility);
}

bool AArenaCharacter::ServerSetUpperBackUtility_Validate(TSubclassOf<class AArenaUtility> Utility)
{
	return true;
}
void AArenaCharacter::ServerSetUpperBackUtility_Implementation(TSubclassOf<class AArenaUtility> Utility)
{
	HandleUpperBackUtility(Utility);
}

bool AArenaCharacter::ServerSetLowerBackUtility_Validate(TSubclassOf<class AArenaUtility> Utility)
{
	return true;
}
void AArenaCharacter::ServerSetLowerBackUtility_Implementation(TSubclassOf<class AArenaUtility> Utility)
{
	HandleLowerBackUtility(Utility);
}

bool AArenaCharacter::ServerSetLeftWristUtility_Validate(TSubclassOf<class AArenaUtility> Utility)
{
	return true;
}
void AArenaCharacter::ServerSetLeftWristUtility_Implementation(TSubclassOf<class AArenaUtility> Utility)
{
	HandleLeftWristUtility(Utility);
}

bool AArenaCharacter::ServerSetRightWristUtility_Validate(TSubclassOf<class AArenaUtility> Utility)
{
	return true;
}
void AArenaCharacter::ServerSetRightWristUtility_Implementation(TSubclassOf<class AArenaUtility> Utility)
{
	HandleRightWristUtility(Utility);
}

bool AArenaCharacter::ServerSetLeftWaistUtility_Validate(TSubclassOf<class AArenaUtility> Utility)
{
	return true;
}
void AArenaCharacter::ServerSetLeftWaistUtility_Implementation(TSubclassOf<class AArenaUtility> Utility)
{
	HandleLeftWaistUtility(Utility);
}

bool AArenaCharacter::ServerSetRightWaistUtility_Validate(TSubclassOf<class AArenaUtility> Utility)
{
	return true;
}
void AArenaCharacter::ServerSetRightWaistUtility_Implementation(TSubclassOf<class AArenaUtility> Utility)
{
	HandleRightWaistUtility(Utility);
}

bool AArenaCharacter::ServerSetHeadArmor_Validate(TSubclassOf<class AArenaArmor> Armor)
{
	return true;
}
void AArenaCharacter::ServerSetHeadArmor_Implementation(TSubclassOf<class AArenaArmor> Armor)
{
	HandleHeadArmor(Armor);
}

bool AArenaCharacter::ServerSetChestArmor_Validate(TSubclassOf<class AArenaArmor> Armor)
{
	return true;
}
void AArenaCharacter::ServerSetChestArmor_Implementation(TSubclassOf<class AArenaArmor> Armor)
{
	HandleChestArmor(Armor);
}

bool AArenaCharacter::ServerSetShoulderArmor_Validate(TSubclassOf<class AArenaArmor> Armor)
{
	return true;
}
void AArenaCharacter::ServerSetShoulderArmor_Implementation(TSubclassOf<class AArenaArmor> Armor)
{
	HandleShoulderArmor(Armor);
}

bool AArenaCharacter::ServerSetHandArmor_Validate(TSubclassOf<class AArenaArmor> Armor)
{
	return true;
}
void AArenaCharacter::ServerSetHandArmor_Implementation(TSubclassOf<class AArenaArmor> Armor)
{
	HandleHandArmor(Armor);
}

bool AArenaCharacter::ServerSetLegArmor_Validate(TSubclassOf<class AArenaArmor> Armor)
{
	return true;
}
void AArenaCharacter::ServerSetLegArmor_Implementation(TSubclassOf<class AArenaArmor> Armor)
{
	HandleLegArmor(Armor);
}

bool AArenaCharacter::ServerSetFeetArmor_Validate(TSubclassOf<class AArenaArmor> Armor)
{
	return true;
}
void AArenaCharacter::ServerSetFeetArmor_Implementation(TSubclassOf<class AArenaArmor> Armor)
{
	HandleFeetArmor(Armor);
}