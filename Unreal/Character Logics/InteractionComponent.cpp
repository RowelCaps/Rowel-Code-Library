// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "InteractableInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ItemUseInterface.h"

#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

// Sets default values for this component's properties
UInteractionComponent::UInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

	if (!bInteractionActive)
		return;

	switch (DetectionType)
	{
		case EDetectionType::PlayerCamera:
			DetectInteractablesFromPlayerCamera();
			break;

		default:
			break;
	}
}

void UInteractionComponent::InteractObject()
{
	if (InteractableDetected.GetActor() == nullptr || 
		!InteractableDetected.GetActor()->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		return;

	IInteractableInterface::Execute_Interact(InteractableDetected.GetActor(), GetOwner(), InteractableDetected);
}

bool UInteractionComponent::LineTraceFromPlayerCamera(FHitResult& OutHit)
{
	APlayerCameraManager* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);

	const FVector Start = PlayerCameraManager->GetCameraLocation();
	const FVector End = (PlayerCameraManager->GetActorForwardVector() * Distance) + Start;

	TArray<AActor*> IgnoredActor;
	IgnoredActor.Add(GetOwner());

	EDrawDebugTrace::Type DebugTrace = bDebugMode ?
		EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	return UKismetSystemLibrary::LineTraceSingle(this, Start, End, TraceChannel, true, IgnoredActor, DebugTrace, OutHit, true);
}

void UInteractionComponent::SetInteractionActive(bool bVal)
{
	bInteractionActive = bVal;

	if (InteractableDetected.GetActor() != nullptr)
	{
		OnEndDetection.Broadcast(InteractableDetected.GetActor());
		InteractableDetected = FHitResult();
	}
}

bool UInteractionComponent::GetUsableItemActor(AActor*& UsableItemActor)
{
	APlayerCameraManager* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);

	const FVector Start = PlayerCameraManager->GetCameraLocation();
	const FVector End = (PlayerCameraManager->GetActorForwardVector() * Distance) + Start;

	TArray<AActor*> IgnoredActor;
	IgnoredActor.Add(GetOwner());

	FHitResult OutHit;

	if (!UKismetSystemLibrary::LineTraceSingle(this, Start, End, UsableActorChannel, true, IgnoredActor, EDrawDebugTrace::None, OutHit, true))
		return false;

	if (OutHit.GetActor()->GetClass()->ImplementsInterface(UItemUseInterface::StaticClass()))
	{
		UsableItemActor = OutHit.GetActor();
		return true;
	}

	return false;
}

void UInteractionComponent::DetectInteractablesFromPlayerCamera()
{
	FHitResult Hit;
	bool DetectedInteractable = LineTraceFromPlayerCamera(Hit);

	//If the previous interactable is the same as the one we're detecting then return immediately
	if (Hit.GetActor() == InteractableDetected.GetActor() && Hit.GetComponent() == InteractableDetected.GetComponent())
		return;

	//If the detected actor is valid then we set the InteractableDetected and call OnStartDetection Interface function
	if (DetectedInteractable && 
		Hit.GetActor()->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
	{
		if (!IInteractableInterface::Execute_IsInteractable(Hit.GetActor()))
			return;

		IInteractableInterface::Execute_OnStartDetection(Hit.GetActor(), GetOwner(), Hit);
		InteractableDetected = Hit;
		OnBeginDetection.Broadcast(Hit.GetActor());

		return;
	}

	//If there is a previous interactable actor then we call OnFinishDetection in interface
	if (InteractableDetected.GetActor() != nullptr)
	{
		IInteractableInterface::Execute_OnFinishDetection(InteractableDetected.GetActor());
		OnEndDetection.Broadcast(InteractableDetected.GetActor());
	}

	//Reset the interactable detected
	InteractableDetected = FHitResult();
}

