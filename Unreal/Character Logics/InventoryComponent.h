// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryData.h"
#include "InventoryComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInventory, Log, All);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THIRDPERSONHORROR_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Slot)
		int32 MaxSlot = 16;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Slot)
		int32 StartSlotCount = 8;

protected:

	TArray<FInventorySlotInfo> Items;

public:

	UFUNCTION(BlueprintPure, Category = Inventory)
		FORCEINLINE TArray<FInventorySlotInfo> GetItems() { return Items; }

	UFUNCTION(BlueprintPure, Category = Inventory)
		FInventorySlotInfo GetSlotByIndex(int32 Index);

	UFUNCTION(BlueprintPure, Category = Inventory)
		TArray<FInventorySlotInfo> FindSlotsByItemClass(TSubclassOf<UItem> ItemClass);

	UFUNCTION(BlueprintPure, Category = Inventory)
		TArray<FInventorySlotInfo> FindSlotsWithUsageType(EItemUsageType UsageType);

	UFUNCTION(BlueprintPure, Category = Inventory)
		TArray<FInventorySlotInfo> FindSlotsWithStoreType(EItemStoreType StoreType);

	UFUNCTION(BlueprintPure, Category = Inventory)
		int32 GetSlotCountWithItem();

	UFUNCTION(BlueprintPure, Category = Inventory)
		int32 GetTotalItemCount();

	UFUNCTION(BlueprintCallable, Category = Inventory)
		bool AddItem(UItem* Item, int32 Count, int32& FailedCount);

	UFUNCTION(BlueprintCallable, Category = Inventory)
		bool AddItemByClass(TSubclassOf<UItem> ItemClass, int32 Count, int32& FailedCount);

	UFUNCTION(BlueprintCallable, Category = Inventory)
		void DiscardItemByClass(TSubclassOf<UItem> ItemClass, int32 Count, int32& FailedCount);

	UFUNCTION(BlueprintCallable, Category = Inventory)
		void DiscardItemByIndex(int32 Index, int32 Count);

	UFUNCTION(BlueprintCallable, Category = Inventory)
		bool UseItem(AActor* User, bool DropItem, int32 Index, int32 Count, int32& FailedUsed);

	UFUNCTION(BlueprintCallable, Category = Inventory)
		bool UseItemByClass(AActor* User, bool DropItem, TSubclassOf<UItem> ItemClass, int32 Count, int32& FailedUsed);

protected:

	bool HasInventorySlot(int32& Index);
	bool GetEmptySlotIndex(int32& Index);


	bool AddToStackableItems(TSubclassOf<UItem> ItemClass, int32 Count, int32& FailedCount);
	bool AddToEmptySlots(UItem* Item, int32 Count, int32& FailedCount);
};
