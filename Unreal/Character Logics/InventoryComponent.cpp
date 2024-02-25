// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Item.h"
#include "ItemDataAsset.h"

DEFINE_LOG_CATEGORY(LogInventory);

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	Items = TArray<FInventorySlotInfo>();
	
	for (int32 i = 0; i < MaxSlot; i++)
	{
		Items.Add(FInventorySlotInfo());
	}
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FInventorySlotInfo UInventoryComponent::GetSlotByIndex(int32 Index)
{
	if (Items.Num() <= Index)
	{
		UE_LOG(LogInventory, Error, TEXT("%d index is out Of bounce. Inventory Slots has %d elements"), 
			Index, Items.Num());

		return FInventorySlotInfo();
	}

	return Items[Index];
}

TArray<FInventorySlotInfo> UInventoryComponent::FindSlotsByItemClass(TSubclassOf<UItem> ItemClass)
{
	return Items.FilterByPredicate([&](FInventorySlotInfo Slot)
			{
				if(Slot.ItemObject.Get() != nullptr)
					return (Slot.ItemObject.Get()->GetClass() == ItemClass);

				return false;
			});
}

TArray<FInventorySlotInfo> UInventoryComponent::FindSlotsWithUsageType(EItemUsageType UsageType)
{
	return Items.FilterByPredicate([&](FInventorySlotInfo Slot)
		{
			if (Slot.ItemObject.Get() != nullptr)
				return (Slot.ItemObject.Get()->ItemData->UsageType == UsageType);

			return false;
		});
}

TArray<FInventorySlotInfo> UInventoryComponent::FindSlotsWithStoreType(EItemStoreType StoreType)
{
	return Items.FilterByPredicate([&](FInventorySlotInfo Slot)
		{
			if (Slot.ItemObject.Get() != nullptr)
				return (Slot.ItemObject.Get()->ItemData->StoreType == StoreType);

			return false;
		});
}

int32 UInventoryComponent::GetSlotCountWithItem()
{
	int32 Count = 0;

	for (FInventorySlotInfo Slot : Items)
	{
		if (Slot.ItemObject.Get() == nullptr || Slot.Count <= 0)
			continue;

		++Count;
	}

	return Count;
}

int32 UInventoryComponent::GetTotalItemCount()
{
	int32 Count = 0;

	for (FInventorySlotInfo Slot : Items)
	{
		if (Slot.ItemObject.Get() == nullptr || Slot.Count <= 0)
			continue;

		Count += Slot.Count;
	}

	return Count;
}

bool UInventoryComponent::AddItem(UItem* Item, int32 Count, int32& FailedCount)
{
	if (Item == nullptr || Count <= 0)
	{
		UE_LOG(LogInventory, Warning, TEXT("Invalid Parameter when adding an Item"));
		return false;
	}

	int32 RemainingItem = Count;
	int32 FailedToAdd = -1;

	switch (Item->ItemData->StoreType)
	{
		case EItemStoreType::NonStackable:

			AddToEmptySlots(Item, RemainingItem, FailedToAdd);

			RemainingItem = FailedToAdd;
			break;

		default:

			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Inventory Add bad");
			if (AddToStackableItems(Item->GetClass(), RemainingItem, FailedToAdd))
				break;

			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Inventory Add sdsd");

			RemainingItem = FailedToAdd;
			FailedToAdd = 0;

			AddToEmptySlots(Item, RemainingItem, FailedToAdd);
			RemainingItem = FailedToAdd;

			break;
	}

	FailedCount = RemainingItem;
	return RemainingItem == Count;
}

bool UInventoryComponent::AddItemByClass(TSubclassOf<UItem> ItemClass, int32 Count, int32& FailedCount)
{
	UItem* Item = NewObject<UItem>(this, ItemClass);
	return AddItem(Item, Count, FailedCount);
}

void UInventoryComponent::DiscardItemByClass(TSubclassOf<UItem> ItemClass, int32 Count, int32& FailedCount)
{
	if (ItemClass == nullptr)
	{
		FailedCount = Count;
		return;
	}

	int32 ItemRemaining = Count;

	for (int32 i = 0; i < Items.Num() && ItemRemaining > 0; i++)
	{
		if (Items[i].IsEmpty())
			continue;

		if (Items[i].ItemObject.Get()->GetClass() == ItemClass)
		{
			int32 DropCount = ItemRemaining <= Items[i].Count ? ItemRemaining : Items[i].Count;
			Items[i].RemoveItem(DropCount);

			ItemRemaining -= DropCount;
		}
	}

	FailedCount = ItemRemaining;
}

void UInventoryComponent::DiscardItemByIndex(int32 Index, int32 Count)
{
	checkf(Index >= 0 && Index <=Items.Num(), TEXT("Index Out Of Bound On Discard Item"));

	Items[Index].RemoveItem(Count);
}

bool UInventoryComponent::UseItem(AActor* User, bool DropItem, int32 Index, int32 Count, int32& FailedUsed)
{
	if (User == nullptr || Index < 0 || Index >= Items.Num())
		return false;

	int32 UsableItemCount = Count > Items[Index].Count ? 
		Items[Index].Count: 
		Count;

	bool SuccessfullyUsed = Items[Index].ItemObject.Get()->UseItem(User, UsableItemCount);
	FailedUsed = UsableItemCount - Count;

	if (DropItem)
		Items[Index].RemoveItem(UsableItemCount);

	return false;
}

bool UInventoryComponent::UseItemByClass(AActor* User, bool DropItem, TSubclassOf<UItem> ItemClass, int32 Count, int32& FailedUsed)
{
	if (User == nullptr || ItemClass == nullptr)
		return false;

	int32 ItemRemaining = Count;

	for (int32 i = 0; i < Items.Num() && ItemRemaining > 0; i++)
	{
		if (Items[i].IsEmpty())
			continue;

		if (Items[i].ItemObject.Get()->GetClass() == ItemClass)
		{
			int32 UseCount = ItemRemaining <= Items[i].Count ? ItemRemaining : Items[i].Count;
			Items[i].ItemObject.Get()->UseItem(User, UseCount);

			if(DropItem)
				Items[i].RemoveItem(UseCount);

			ItemRemaining -= UseCount;
		}
	}	

	FailedUsed = ItemRemaining;
	return ItemRemaining < Count;
}

bool UInventoryComponent::HasInventorySlot(int32& Index)
{
	int32 CurrentIndex = 0;

	return Items.ContainsByPredicate([&](FInventorySlotInfo Info) 
		{
			if (Info.IsEmpty())
			{
				Index = CurrentIndex;
				return true;
			}

			return false;
		});
}

bool UInventoryComponent::GetEmptySlotIndex(int32& Index)
{
	int32 CurrentIndex = 0;

	return Items.ContainsByPredicate([&](FInventorySlotInfo Info)
		{
			if (Info.IsEmpty())
			{
				Index = CurrentIndex;
				return true;
			}

			CurrentIndex++;
			return false;
		});
}

bool UInventoryComponent::AddToStackableItems(TSubclassOf<UItem> ItemClass, int32 Count, int32& FailedCount)
{
	int32 RemainingCount = Count;

	for (int i = 0; i < Items.Num(); i++)
	{
		if (Items[i].IsFull() || Items[i].IsEmpty())
			continue;

		UItemDataAsset* DataAsset = Items[i].ItemObject.Get()->ItemData;

		if (Items[i].ItemObject.Get()->GetClass() == ItemClass &&
			Items[i].Count < DataAsset->MaxStacks)
		{
			const int32 AddCount = DataAsset->MaxStacks - Items[i].Count;
			Items[i].Count += AddCount;

			RemainingCount -= AddCount;
		}
	}

	FailedCount = RemainingCount;
	return RemainingCount <= 0;
}

bool UInventoryComponent::AddToEmptySlots(UItem* Item,int32 Count, int32& FailedCount)
{
	int32 RemainingCount = Count;
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Inventory Add shit");

	for (int i = 0; i < Items.Num() && RemainingCount > 0; i++)
	{
		if (!Items[i].IsEmpty())
			continue;

		UItemDataAsset* DataAsset = Item->ItemData;
		const int32 MaxStacks = DataAsset->StoreType == EItemStoreType::Stackable ? 
			DataAsset->MaxStacks : 1;

		const int32 AddCount = FMath::Clamp(RemainingCount, 1, MaxStacks);
		Items[i].AddItem(Item, AddCount);

		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Inventory Add " + FString::FromInt(AddCount));

		RemainingCount -= AddCount;
	}

	FailedCount = RemainingCount;
	return RemainingCount <= 0;
}
