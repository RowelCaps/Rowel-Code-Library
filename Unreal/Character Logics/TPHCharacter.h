// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CharacterMovementData.h"
#include "TPHCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UCurveFloat;
class UInteractorComponent;
class UInventoryComponent;
class UWeapon;
class UEquippableItem;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGaitSignature, EGaitType, GaitType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStanceSignature, EStanceType, StanceType);

DECLARE_LOG_CATEGORY_EXTERN(LogTPHCharacter, Log, All);

UCLASS()
class THIRDPERSONHORROR_API ATPHCharacter : public ACharacter
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = Camera, meta = (AllowPrivateAccess = true))
		USpringArmComponent* CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = Camera, meta = (AllowPrivateAccess = true))
		UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = Camera, meta = (AllowPrivateAccess = true))
		UInteractorComponent* InteractorComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = Camera, meta = (AllowPrivateAccess = true))
		UInventoryComponent* InventoryComponent;

public:

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

#pragma region Inputs

protected:

	//======================Input=====================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = Input, meta = (AllowPrivateAccess = true))
		UInputMappingContext* CharacterMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = Input, meta = (AllowPrivateAccess = true))
		UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = Input, meta = (AllowPrivateAccess = true))
		UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = Input, meta = (AllowPrivateAccess = true))
		UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = Input, meta = (AllowPrivateAccess = true))
		UInputAction* RunAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = Input, meta = (AllowPrivateAccess = true))
		UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = Input, meta = (AllowPrivateAccess = true))
		UInputAction* OpenInventoryAction;

#pragma endregion Inputs

public:
	// Sets default values for this character's properties
	ATPHCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

#pragma region Movement

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Movement)
		float WalkSpeed = 120.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Movement)
		float RunSpeed = 250.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Movement)
		float AimSpeed = 250.0f;

	UPROPERTY(BlueprintAssignable, Category = TPHCharacter)
		FGaitSignature OnChangedGait;

	UPROPERTY(BlueprintAssignable, Category = TPHCharacter)
		FStanceSignature OnChangedStance;

protected:

	EGaitType GaitType;
	EStanceType StanceType;

protected:

	void Run();
	void StopRun();

public:

	UFUNCTION(BlueprintPure, Category = "TPHCharacter")
		FORCEINLINE EGaitType GetGaitType() const { return GaitType; }

	UFUNCTION(BlueprintPure, Category = "TPHCharacter")
		FORCEINLINE EStanceType GetStanceType() const { return StanceType; }

	UFUNCTION(BlueprintCallable, Category = "TPHCharacter")
		void SetGaitType(EGaitType NewGait);

	UFUNCTION(BlueprintCallable, Category = "TPHCharacter")
		void SetStanceType(EStanceType NewStance);

#pragma endregion Movement

#pragma region Aim

protected:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, category = Aim)
		float AimDuration = 0.5f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, category = Aim)
		float AimTargetArmLength = 50.0f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, category = Aim)
		FVector AimSocketTargetOffset = FVector(0, 200, 0);

	UPROPERTY(BlueprintReadOnly, EditAnywhere, category = Aim)
		UCurveFloat* AimCurve;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, category = Aim)
		float MaxCastDistance = 1000;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Interactor)
		TArray<TEnumAsByte< EObjectTypeQuery>> ObjectQuery;

protected:

	bool bEnableAiming = true;
	bool bIsAiming = false;

	float InitialCameraBoomArmlength = 0.0f;
	FVector InitialCameraBoomSocketTargetOffset;

	float OnLerpCameraBoomArmLength = 0.0f;
	FVector OnLerpCameraBoomSocketTargetOffset;

	float AimElapsedTime = 0.0f;


public:

	UFUNCTION(BlueprintPure, Category = "TPHCharacter")
		FORCEINLINE bool IsAiming() const { return bIsAiming; }

	UFUNCTION(BlueprintCallable, Category = "TPHCharacter")
		void EnableAiming(bool Value);

	UFUNCTION(BlueprintCallable, Category = "TPHCharacter")
		bool GetAimedObject(FHitResult& Hit);

protected:

	void Aim();
	void CancelAim();

	void OnPressedActionButton();


private:

	void AimCameraBoomLerp(float TargetCameraLength, FVector TargetSocketOffset, float DeltaTime);

#pragma endregion Aim

#pragma region Attack/Weapon Logic

protected:

	TWeakObjectPtr<UWeapon> EquippedWeapon;
	TArray<TWeakObjectPtr<UEquippableItem>> EquippedItems;

protected:

	void Attack();

public:

	UFUNCTION(BlueprintCallable, Category = TPHCharacter)
		void EquipWeapon(UWeapon* Weapon);

	UFUNCTION(BlueprintCallable, Category = TPHCharacter)
		void EquipItem(UEquippableItem* Item);

	UFUNCTION(BlueprintCallable, Category = TPHCharacter)
		void UnequipWeapon();

	UFUNCTION(BlueprintCallable, Category = TPHCharacter)
		void UnequipItem(UEquippableItem* Item);

	UFUNCTION(BlueprintPure, Category = TPHCharacter)
		FORCEINLINE UWeapon* GetEquippedWeapon() { return EquippedWeapon.Get(); }

	UFUNCTION(BlueprintPure, Category = TPHCharacter)
		TArray<UEquippableItem*> GetEquippedItems();

	UFUNCTION(BlueprintPure, Category = TPHCharacter)
		bool ContainsEquippedItemOfClass(TSubclassOf<UEquippableItem> ItemClass);

	UFUNCTION(BlueprintPure, Category = TPHCharacter)
		UEquippableItem* GetItemfOfClass(TSubclassOf<UEquippableItem> ItemClass);

#pragma endregion

#pragma region UI

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UI")
		TSubclassOf<UUserWidget> InventoryWidgetClass;

protected:

	UUserWidget* InventoryWidget;

	void OpenInventory();

public:

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = TPHCharacter)
		void OnOpenInventory(UUserWidget* Widget);
	void OnOpenInventory_Implementation(UUserWidget* Widget);

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = TPHCharacter)
		void CloseInventory();
	virtual void CloseInventory_Implementation();

	UFUNCTION(BlueprintPure, Category = TPHCharacter)
		UUserWidget* GetInventoryWidget() const { return InventoryWidget; }


#pragma endregion

#if WITH_EDITORONLY_DATA

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Interactor)
		bool bDebugInteractorComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Interactor)
		FColor DebugInteractorColor;

	virtual bool ShouldTickIfViewportsOnly() const override;	
	void DrawInteractorRadius();

#endif
};
