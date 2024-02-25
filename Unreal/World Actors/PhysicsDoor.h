// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoorData.h"
#include "InteractableInterface.h"
#include "ItemUseInterface.h"
#include "SaveLoadActorInterface.h"
#include "PhysicsDoor.generated.h"

class UPhysicsConstraintComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UItem;
class ASmartNavLinkProxy;
class UAudioComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDoorSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDoorStateSignature, EDoorState, State);

UCLASS()
class SHADOWOFTHEOTHERSIDE_API APhysicsDoor : public AActor,public IInteractableInterface, public IItemUseInterface, public ISaveLoadActorInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APhysicsDoor();

protected:

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		USceneComponent* RootScene;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		UStaticMeshComponent* Door1Mesh;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		UBoxComponent* DoorCollisionHull;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		UPhysicsConstraintComponent* Door1Constraint;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		UAudioComponent* AudioComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Settings: ")
		ASmartNavLinkProxy* NavLink;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Settings: ", meta = (ClampMin = "0", ClampMax = "90"))
		float CloseAngle = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Settings: ", meta = (ClampMin = "0", ClampMax = "90"))
		float PartialOpenAngle = 45;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Settings: ", meta = (ClampMin = "0", ClampMax = "90"))
		float OpenAngle = 90;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Settings: ", meta = (ClampMin = "0"))
		float RotationCompareTolerance = 0.5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Update Settings: ", meta = (ClampMin = "0"))
		float OpenRange = 45;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Update Settings: ", meta = (ClampMin = "0"))
		float CloseRange = 30;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Update Settings: ", meta = (ClampMin = "0"))
		float CloseSnapRange = 20.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Start Settings: ")
		EDoorState StartingState;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Start Settings: ")
		float StartOpenAngle = 90;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Start Settings: ")
		float StartPartialOpenAngle = 45;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Lock Settings: ")
		UItem* Key;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Footstep Settings:")
		USoundAttenuation* DoorAttenuation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Audio")
		USoundBase* OpenSound;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Audio")
		USoundBase* CloseSound;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Audio")
		USoundBase* LockSound;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Door Audio")
		USoundBase* UnlockSound;

public:

	UPROPERTY(BlueprintAssignable)
		FDoorSignature OnUnlockedDoor;

	UPROPERTY(BlueprintAssignable)
		FDoorStateSignature OnChangedState;

	UPROPERTY(BlueprintAssignable)
		FDoorSignature OnLockDoor;

	UPROPERTY(BlueprintAssignable)
		FDoorSignature OnCloseSnapDoor;

	UPROPERTY(BlueprintAssignable)
		FDoorSignature OnInteractDoor;

public:

	bool bAutoRotate = false;

	UPROPERTY(SaveGame)
		bool bContainsSaveData = false;

	UPROPERTY(SaveGame)
		int32 ConvertedDoorState = 0;

	UPROPERTY(SaveGame, BlueprintReadWrite)
		EDoorState DoorState;

protected:

	float TargetAngle = 0;

	//Used to detect if the door mesh has reached the closing angle
	bool bFinishedClosing = false;

	FTimerHandle CloseSnapHandle;

	bool bCanClose = false;

	FRotator InitialRotation;

public:

	UFUNCTION(BlueprintCallable, Category = "Door Settings: ")
		void SetDoorState(EDoorState State, FVector SwingDirection = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category = "Physics Door")
		void SetNavLinkEnabled(bool Val);

	UFUNCTION(BlueprintPure, Category = "Physics Door")
		FRotator GetTargetOrientation();

	UFUNCTION(BlueprintPure, Category = "Physics Door")
		FORCEINLINE FRotator GetInitialRotation() { return InitialRotation; }

	UFUNCTION(BlueprintPure, Category = "Physics Door")
		FORCEINLINE EDoorState GetDoorState() { return DoorState; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Physics Door")
		void OnInteractLockDoor(AActor* Interactor);

#if WITH_EDITOR

	UFUNCTION(CallInEditor, Category = "Door Settings: ")
		void SpawnNavLinkProxy();

#endif

	//Interact Overide funtion of interaction interface
	virtual void Interact_Implementation(AActor* Interactor, FHitResult Hit) override;

	virtual bool IsInteractable_Implementation() override;

	virtual bool OnUsedItem_Implementation(UItem* Item, class AHumanoidCharacter* User) override;

	void DoorUpdateRotation();

	void InitializeDoor(EDoorState State);

	UFUNCTION()
			void CloseSnapEnd();

	UFUNCTION()
	void OnCollisionHullOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	FRotator GetDoorRelativeRotation();

	void PlaySoundAtDoor(USoundBase* Sound);

	public:

		virtual void OnActorSave_Implementation() override;
		virtual void OnActorLoaded_Implementation() override;
};	
