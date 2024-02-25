// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "AnimationSynchingInterface.h"
#include "Door.generated.h"

class UStaticMeshComponent;
class UItem;
class UArrowComponent;

UENUM(BlueprintType)
enum class EOldDoorState : uint8
{	
	Open,
	Close
};

UENUM(BlueprintType)
enum class EDoorInteractMode: uint8
{
	Unlocked,
	Lock	
};

UCLASS()
class SHADOWOFTHEOTHERSIDE_API ADoor : public AActor, public IInteractableInterface, public IAnimationSynchingInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		USceneComponent* RootScene;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		UStaticMeshComponent* DoorFrameMesh;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		UStaticMeshComponent* DoorMesh;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		UArrowComponent* ForwardInteractLocation;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		UArrowComponent* BackInteractLocation;

public:

	FORCEINLINE UStaticMeshComponent* GetDoorFrameMesh() { return DoorFrameMesh; }
	FORCEINLINE UStaticMeshComponent* GetDoorMesh() { return DoorMesh; }

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DoorSettings: ")
		UItem* Key;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DoorSettings: ")
		FName RotationCurveName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Animation:|Open Montage:")
		UAnimMontage* OpenForwardMontage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Animation:|Open Montage:")
		UAnimMontage* OpenBackMontage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Animation:|Close Montage:")
		UAnimMontage* CloseForwardMontage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Animation:|Close Montage:")
		UAnimMontage* CloseBackMontage;

protected:

	EOldDoorState State;
	EDoorInteractMode InteractMode;

public:

	UFUNCTION(BlueprintPure, Category = "Door")
		FORCEINLINE EOldDoorState GetDoorState() { return State; }

	UFUNCTION(BlueprintPure, Category = "Door")
		FORCEINLINE EDoorInteractMode GetDoorInteractMode() { return InteractMode; }


public:

	virtual void Interact_Implementation(AActor* Interactor, FHitResult Hit);
	virtual bool IsInteractable_Implementation();

	virtual void SynchUpdate_Implementation(FAnimationSynchReceiverData CurveData, AActor* Pair);
	virtual void OnEndSyching_Implementation(AActor* Pair);

protected:

	//Get the appropriate montage relative to the interactors Location
	UAnimMontage* GetAnimMontageFromLocation(FVector Location);

	//Gets where the character should start when playing montage
	void GetInteractLocationRotation(FVector CharacterLocation, FVector &InteractLocation, FRotator& InteractRotation);
};
