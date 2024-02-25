// Fill out your copyright notice in the Description page of Project Settings.


#include "Door.h"
#include "Components/StaticMeshComponent.h"
#include "HumanoidCharacter.h"
#include "Components/ArrowComponent.h"

// Sets default values
ADoor::ADoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootScene);

	DoorFrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
	DoorFrameMesh->SetupAttachment(RootScene);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door"));
	DoorMesh->SetupAttachment(DoorFrameMesh);

	ForwardInteractLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("Forward Interact Location"));
	ForwardInteractLocation->SetupAttachment(RootScene);

	BackInteractLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("Back Interact Location"));
	BackInteractLocation->SetupAttachment(RootScene);

	ForwardInteractLocation->ArrowSize = 0.5;
	BackInteractLocation->ArrowSize = 0.5;
}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADoor::Interact_Implementation(AActor* Interactor, FHitResult Hit)
{
	AHumanoidCharacter* Humanoid = Cast<AHumanoidCharacter>(Interactor);

	if (Humanoid == nullptr)
		return;

	switch (InteractMode)
	{
		case EDoorInteractMode::Unlocked:

			UAnimMontage* Montage = GetAnimMontageFromLocation(Humanoid->GetActorLocation());
			TArray<FName> Curves;
			Curves.Add("DoorOpen");
			FVector InteractLocation;
			FRotator InteractRotation;

			GetInteractLocationRotation(Humanoid->GetActorLocation(), InteractLocation, InteractRotation);

			FAnimationSynchParams SynchParams = FAnimationSynchParams(
				Montage,
				InteractLocation,
				InteractRotation,
				Curves,
				this);

			DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Humanoid->StartAnimationSynching(SynchParams);

			break;
	}
}

bool ADoor::IsInteractable_Implementation()
{
	return true;
}

void ADoor::SynchUpdate_Implementation(FAnimationSynchReceiverData CurveData, AActor* Pair)
{
	if (CurveData.CurveName != FName("DoorOpen"))
		return;
		
	FRotator NewDoorRot = FRotator(0, CurveData.CurveValue, 0);
	DoorMesh->SetRelativeRotation(NewDoorRot);
}

void ADoor::OnEndSyching_Implementation(AActor* Pair)
{
	AHumanoidCharacter* Humanoid = Cast<AHumanoidCharacter>(Pair);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	if (Humanoid == nullptr)
		return;

	Humanoid->SetActionMode(EActionMode::Moving);
	State = State == EOldDoorState::Open ? EOldDoorState::Close : EOldDoorState::Open;
}

UAnimMontage* ADoor::GetAnimMontageFromLocation(FVector Location)
{
	UAnimMontage* MontageToPlay;

	FVector Dir = Location - GetActorLocation();
	Dir.Normalize();

	float Dot = FVector::DotProduct(GetActorForwardVector(), Dir);

	switch (State)
	{
		case EOldDoorState::Open:
			MontageToPlay = Dot > 0 ? OpenForwardMontage : OpenBackMontage;
			break;
		
		default:
			MontageToPlay = Dot > 0 ? CloseForwardMontage : CloseBackMontage;
			break;
	}

	return MontageToPlay;
}

void ADoor::GetInteractLocationRotation(FVector CharacterLocation, FVector& InteractLocation, FRotator& InteractRotation)
{
	FVector Dir = CharacterLocation - GetActorLocation();
	Dir.Normalize();

	float Dot = FVector::DotProduct(GetActorForwardVector(), Dir);

	if (Dot > 0)
	{
		InteractLocation = ForwardInteractLocation->GetComponentLocation();
		InteractRotation = ForwardInteractLocation->GetComponentRotation();

		return;
	}

	InteractLocation = BackInteractLocation->GetComponentLocation();
	InteractRotation = BackInteractLocation->GetComponentRotation();
}

