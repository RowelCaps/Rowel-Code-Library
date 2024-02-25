// Fill out your copyright notice in the Description page of Project Settings.


#include "TPHCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Curves/CurveFloat.h"
#include "Kismet/KismetMathLibrary.h"
#include "InteractorComponent.h"
#include "InventoryComponent.h"
#include "InteractableInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/PlayerCameraManager.h"
#include "EquippableItem.h"
#include "Blueprint/UserWidget.h"

DEFINE_LOG_CATEGORY(LogTPHCharacter);

// Sets default values
ATPHCharacter::ATPHCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	InteractorComponent = CreateDefaultSubobject<UInteractorComponent>(TEXT("Interactor Component"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory Component"));
}

// Called when the game starts or when spawned
void ATPHCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			SubSystem->AddMappingContext(CharacterMappingContext, 0);
		}
	}

	InitialCameraBoomArmlength = CameraBoom->TargetArmLength;
	InitialCameraBoomSocketTargetOffset = CameraBoom->SocketOffset;
}

// Called every frame
void ATPHCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if WITH_EDITOR

	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Editor)
	{
		DrawInteractorRadius();
		return;
	}

#endif

	if (bEnableAiming && AimElapsedTime < AimDuration)
	{

		if (bIsAiming)
		{
			if (!FMath::IsNearlyEqual(CameraBoom->TargetArmLength, AimTargetArmLength))
				AimCameraBoomLerp(AimTargetArmLength, AimSocketTargetOffset, DeltaTime);
		}
		else
		{
			if (!FMath::IsNearlyEqual(CameraBoom->TargetArmLength, InitialCameraBoomArmlength))
				AimCameraBoomLerp(InitialCameraBoomArmlength, InitialCameraBoomSocketTargetOffset, DeltaTime);
		}
	}
}

// Called to bind functionality to input
void ATPHCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnchancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnchancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATPHCharacter::Move);
		EnchancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATPHCharacter::Look);

		EnchancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ATPHCharacter::Aim);
		EnchancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ATPHCharacter::CancelAim);

		EnchancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ATPHCharacter::OnPressedActionButton);

		EnchancedInputComponent->BindAction(RunAction, ETriggerEvent::Triggered, this, &ATPHCharacter::Run);
		EnchancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ATPHCharacter::StopRun);

		EnchancedInputComponent->BindAction(OpenInventoryAction, ETriggerEvent::Triggered, this, &ATPHCharacter::OpenInventory);
	}

	InitialCameraBoomArmlength = CameraBoom->TargetArmLength;
	InitialCameraBoomSocketTargetOffset = CameraBoom->SocketOffset;
}

void ATPHCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATPHCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ATPHCharacter::Run()
{
	SetGaitType(EGaitType::Running);
}

void ATPHCharacter::StopRun()
{
	SetGaitType(EGaitType::Walking);
}

void ATPHCharacter::SetGaitType(EGaitType NewGait)
{
	if (GaitType == NewGait)
		return;

	GaitType = NewGait;

	switch (GaitType)
	{
		case EGaitType::Walking:
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			break;

		case EGaitType::Running:
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
			break;

		case EGaitType::Aiming:
			GetCharacterMovement()->MaxWalkSpeed = AimSpeed;
			break;
	}

	OnChangedGait.Broadcast(GaitType);
}

void ATPHCharacter::SetStanceType(EStanceType NewStance)
{
	if (NewStance == StanceType)
		return;

	StanceType = NewStance;

	switch (StanceType)
	{
		case EStanceType::Standing:
			if (bIsCrouched)
				UnCrouch();
			break;

		case EStanceType::Crouching:
			if (!bIsCrouched)
				Crouch();
			break;
	}

	OnChangedStance.Broadcast(StanceType);
}

void ATPHCharacter::EnableAiming(bool Value)
{
	bEnableAiming = Value;
}

bool ATPHCharacter::GetAimedObject(FHitResult& Hit)
{
	UWorld* World = GetWorld();

	if (World == nullptr)
		return false;

	APlayerCameraManager* PlayerCamera = World->GetFirstPlayerController()->PlayerCameraManager;

	const FVector CamLocation = PlayerCamera->GetCameraLocation();
	const FVector CamForward = FRotationMatrix(PlayerCamera->GetCameraRotation()).GetUnitAxis(EAxis::X);
	const FVector Endloc = CamLocation + (CamForward * MaxCastDistance);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	return UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		CamLocation,
		Endloc,
		ObjectQuery,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		Hit,
		true);
}

void ATPHCharacter::Aim()
{
	if (bIsAiming)
		return;

	bIsAiming = true;

	AimElapsedTime = 0;

	OnLerpCameraBoomArmLength = CameraBoom->TargetArmLength;
	OnLerpCameraBoomSocketTargetOffset = CameraBoom->SocketOffset;
}

void ATPHCharacter::CancelAim()
{
	if (!bIsAiming)
		return;

	bIsAiming = false;

	AimElapsedTime = 0;

	OnLerpCameraBoomArmLength = CameraBoom->TargetArmLength;
	OnLerpCameraBoomSocketTargetOffset = CameraBoom->SocketOffset;
}

void ATPHCharacter::AimCameraBoomLerp(float TargetCameraLength, FVector TargetSocketOffset, float DeltaTime)
{
	AimElapsedTime += DeltaTime;

	const float Alpha = AimCurve->GetFloatValue((AimElapsedTime / AimDuration));

	float NextTargetArmLength = UKismetMathLibrary::Lerp(OnLerpCameraBoomArmLength, TargetCameraLength, Alpha);
	FVector NextSocketOffset =UKismetMathLibrary::VLerp(OnLerpCameraBoomSocketTargetOffset, TargetSocketOffset, Alpha);

	CameraBoom->TargetArmLength = NextTargetArmLength;
	CameraBoom->SocketOffset = NextSocketOffset;
}

void ATPHCharacter::OnPressedActionButton()
{
	if (bIsAiming)
	{
		Attack();
		return;
	}

	AActor* DetectedInteractable = InteractorComponent->GetCurrentDetectedInteractable();

	if (DetectedInteractable == nullptr)
		return;

	IInteractableInterface::Execute_Interact(DetectedInteractable, this);
}

void ATPHCharacter::Attack()
{
}

void ATPHCharacter::EquipWeapon(UWeapon* Weapon)
{
}

void ATPHCharacter::EquipItem(UEquippableItem* Item)
{
}

void ATPHCharacter::UnequipWeapon()
{
}

void ATPHCharacter::UnequipItem(UEquippableItem* Item)
{
}

TArray<UEquippableItem*> ATPHCharacter::GetEquippedItems()
{
	return TArray<UEquippableItem*>();
}

bool ATPHCharacter::ContainsEquippedItemOfClass(TSubclassOf<UEquippableItem> ItemClass)
{
	return false;
}

UEquippableItem* ATPHCharacter::GetItemfOfClass(TSubclassOf<UEquippableItem> ItemClass)
{
	return nullptr;
}

void ATPHCharacter::OpenInventory()
{
	if (InventoryWidgetClass == nullptr)
	{
		UE_LOG(LogTPHCharacter, Error, TEXT("Inventory Widget Class is Null"));
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	InventoryWidget = CreateWidget<UUserWidget>(PlayerController, InventoryWidgetClass);
	DisableInput(PlayerController);

	OnOpenInventory(InventoryWidget);
	InventoryWidget->AddToViewport();
}

void ATPHCharacter::OnOpenInventory_Implementation(UUserWidget* Widget)
{
}

void ATPHCharacter::CloseInventory_Implementation()
{
}

#if WITH_EDITOR

bool ATPHCharacter::ShouldTickIfViewportsOnly() const
{
	if (!bDebugInteractorComponent)
		return false;

	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Editor)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ATPHCharacter::DrawInteractorRadius()
{
	DrawDebugSphere(GetWorld(), GetActorLocation(), InteractorComponent->InteractionRadius, 12, DebugInteractorColor, false, 0);
}

#endif
