// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsDoor.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "HumanoidCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "SmartNavLinkProxy.h"
#include "NavLinkCustomComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

// Sets default values
APhysicsDoor::APhysicsDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootScene);

	Door1Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door1"));
	Door1Mesh->SetupAttachment(RootScene);
	Door1Mesh->SetSimulatePhysics(true);

	DoorCollisionHull = CreateDefaultSubobject<UBoxComponent>(TEXT("Door Collision Hull"));
	DoorCollisionHull->SetupAttachment(Door1Mesh);

	Door1Constraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("Door 1 Constraint"));
	Door1Constraint->SetupAttachment(RootScene);

	Door1Constraint->ComponentName1.ComponentName = FName("Door1");

	Door1Constraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, 90);
	Door1Constraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 45);
	Door1Constraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 45);

	Door1Constraint->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Door Audio"));
	AudioComponent->SetupAttachment(Door1Mesh);

}

// Called when the game starts or when spawned	
void APhysicsDoor::BeginPlay()
{
	DoorCollisionHull->OnComponentBeginOverlap.AddDynamic(this, &APhysicsDoor::OnCollisionHullOverlap);
	InitialRotation = Door1Mesh->GetComponentRotation();

	Super::BeginPlay(); 
}

// Called every frame
void APhysicsDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DoorUpdateRotation();
}

void APhysicsDoor::SetDoorState(EDoorState State, FVector SwingDirection)
{
	float Direction = FVector::DotProduct(SwingDirection, GetActorForwardVector()) > 0 ? 1 : -1;
	FRotator AngularTarget;

	switch (State)
	{
		case EDoorState::Open:
			AngularTarget = FRotator(0, OpenAngle * Direction,0);
			SetNavLinkEnabled(true);

			if (DoorState == EDoorState::Close)
				PlaySoundAtDoor(OpenSound);

			break;

		case EDoorState::PartiallyOpen:
			AngularTarget = FRotator(0, PartialOpenAngle * Direction, 0);
			bAutoRotate = true;
			SetNavLinkEnabled(true);

			if (DoorState != EDoorState::Locked)
				PlaySoundAtDoor(OpenSound);
			break;

		case EDoorState::Locked:
			AngularTarget = FRotator(0, CloseAngle, 0);
			bAutoRotate = true;
			SetNavLinkEnabled(false);
			break;

		default:
			AngularTarget = FRotator(0, CloseAngle, 0);
			SetNavLinkEnabled(true);
			break;
	}


	if (DoorState == EDoorState::Locked && State != EDoorState::Locked)
		Door1Mesh->SetSimulatePhysics(true);

	DoorState = State;
	Door1Constraint->SetAngularOrientationTarget(AngularTarget);

	if(Door1Mesh->IsSimulatingPhysics())
		Door1Mesh->AddAngularImpulseInDegrees(FVector::ZeroVector);

	OnChangedState.Broadcast(DoorState);
}

void APhysicsDoor::SetNavLinkEnabled(bool Val)
{
	if (NavLink == nullptr)
		return;

	NavLink->SetSmartLinkEnabled(Val);
}

FRotator APhysicsDoor::GetTargetOrientation()
{
	switch (DoorState)
	{
		case EDoorState::Open:
			return FRotator(0, OpenAngle, 0);

		case EDoorState::PartiallyOpen:
			return FRotator(0, PartialOpenAngle, 0);

		default:
			return FRotator(0, CloseAngle, 0);
	}
}

#if WITH_EDITOR
void APhysicsDoor::SpawnNavLinkProxy()
{
	if (NavLink != nullptr)
		return;

	NavLink = GetWorld()->SpawnActor<ASmartNavLinkProxy>(
		ASmartNavLinkProxy::StaticClass(),
		GetActorLocation(),
		GetActorRotation());

	NavLink->AttachToActor(this,FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	NavLink->OwningLinkActor = this;
}
#endif

void APhysicsDoor::Interact_Implementation(AActor* Interactor, FHitResult Hit)
{
	if (Interactor == nullptr)
		return;


	FVector ActorLoc = Interactor->GetActorLocation();
	ActorLoc.Z = GetActorLocation().Z;

	FVector Dir = GetActorLocation() - ActorLoc;
	Dir.Normalize();

	OnInteractDoor.Broadcast();

	switch (DoorState)
	{
		case EDoorState::Open:
			SetDoorState(EDoorState::Close, Dir);
			break;

		case EDoorState::Close:
			SetDoorState(EDoorState::PartiallyOpen, Dir);
			break;

		case EDoorState::PartiallyOpen:
			SetDoorState(EDoorState::Open, Dir);
			break;

		default:

			SetDoorState(EDoorState::Locked);
			PlaySoundAtDoor(LockSound);
			OnInteractLockDoor(Interactor);
			return;
	}

	bAutoRotate = true;
}

bool APhysicsDoor::IsInteractable_Implementation()
{
	return true;
}

bool APhysicsDoor::OnUsedItem_Implementation(UItem* Item, AHumanoidCharacter* User)
{
	if (Item == nullptr || DoorState != EDoorState::Locked)
		return false;

	FVector ActorLoc = User->GetActorLocation();
	ActorLoc.Z = GetActorLocation().Z;

	FVector Dir = GetActorLocation() - ActorLoc;
	Dir.Normalize();

	SetDoorState(EDoorState::PartiallyOpen, Dir);
	bAutoRotate = true;

	PlaySoundAtDoor(UnlockSound);
	OnUnlockedDoor.Broadcast();

	return true;
}

void APhysicsDoor::DoorUpdateRotation()
{
	if (!Door1Mesh->IsSimulatingPhysics())
		return;

	FRotator RelativeRotation = GetDoorRelativeRotation();

	if (!bCanClose)
	{
		if (FMath::Abs(RelativeRotation.Yaw) >= CloseSnapRange)
			bCanClose = true;
	}

	if (RelativeRotation.Equals(FRotator(0, CloseAngle, 0), RotationCompareTolerance) && bCanClose)
	{
		Door1Constraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 90);

		PlaySoundAtDoor(CloseSound);
		GetWorld()->GetTimerManager().SetTimer(CloseSnapHandle, this, &APhysicsDoor::CloseSnapEnd, 0.01);
		bCanClose = false;

		if(DoorState != EDoorState::Locked)
			SetDoorState(EDoorState::Close);
		else
		{
			Door1Mesh->SetSimulatePhysics(false);
			OnLockDoor.Broadcast();
		}
	}

	if (bAutoRotate)	
	{
		FRotator Target = GetTargetOrientation() * (RelativeRotation.Yaw > 0 ? 1: -1);

		if (Target.Equals(RelativeRotation, 1.0f))
			bAutoRotate = false;

		return;
	}

	switch (DoorState)
	{
		case EDoorState::Locked:
			break;

		case EDoorState::Close:

			if (FMath::Abs(RelativeRotation.Yaw) >= OpenRange)
			{
				FVector Direction = RelativeRotation.Yaw > 0 ? -GetActorForwardVector() : GetActorForwardVector();
				SetDoorState(EDoorState::Open, Direction);
			}

			break;

		default:

			if (FMath::Abs(RelativeRotation.Yaw) <= CloseRange)
			{
				FVector Direction = RelativeRotation.Yaw > 0 ? -GetActorForwardVector() : GetActorForwardVector();
				SetDoorState(EDoorState::Close, Direction);
			}

	}
}

void APhysicsDoor::InitializeDoor(EDoorState State)
{
	FRotator StartRotation;
	DoorState = State;

	switch (State)
	{
		case EDoorState::Open:
			StartRotation = FRotator(0, OpenAngle, 0);
			bCanClose = true;
			Door1Mesh->SetSimulatePhysics(true);
			break;

		case EDoorState::PartiallyOpen:
			StartRotation = FRotator(0, PartialOpenAngle, 0);
			bCanClose = true;
			Door1Mesh->SetSimulatePhysics(true);
			break;

		case EDoorState::Locked:

			StartRotation = FRotator(0, CloseAngle, 0);
			Door1Mesh->SetSimulatePhysics(false);

			break;
		
		default:
			StartRotation = FRotator(0, CloseAngle, 0);
			Door1Mesh->SetSimulatePhysics(true);
			break;
	}

	Door1Mesh->SetWorldRotation(UKismetMathLibrary::ComposeRotators(StartRotation, GetActorRotation()));
	Door1Constraint->SetAngularOrientationTarget(StartRotation);
}

void APhysicsDoor::CloseSnapEnd()
{
	Door1Constraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, 90);
	OnCloseSnapDoor.Broadcast();
}

void APhysicsDoor::OnCollisionHullOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(Cast<ACharacter>(OtherActor) != nullptr)
		bAutoRotate = false;
}

FRotator APhysicsDoor::GetDoorRelativeRotation()
{
	return UKismetMathLibrary::NormalizedDeltaRotator(GetActorRotation(), Door1Mesh->GetComponentRotation());
}

void APhysicsDoor::PlaySoundAtDoor(USoundBase* Sound)
{
	if (Sound == nullptr)
		return;

	AudioComponent->SetSound(Sound);
	AudioComponent->Play();
}

void APhysicsDoor::OnActorSave_Implementation()
{
	
	bContainsSaveData = true;
	ConvertedDoorState = (int32)DoorState;
}

void APhysicsDoor::OnActorLoaded_Implementation()
{
	if (!bContainsSaveData)
	{
		InitializeDoor(StartingState);
	}
	else
	{
		DoorState = (EDoorState)ConvertedDoorState;
		InitializeDoor(DoorState);
	}
}
