using System.Collections;
using System.Collections.Generic;
using System;
using UnityEngine;
using UnityEngine.Events;
using UnityEngine.Assertions;
using SimpleJSON;

[Serializable]
public class InventorySignatureEvent : UnityEvent<Inventory, Item> { }

[Serializable]
public class InventoryModifySignatureEvent : UnityEvent<Inventory, Item, int> { }

public class Inventory : MonoBehaviour, ISaveableComponent
{
    [Header("Inventory Settings")]
    [SerializeField] bool AutomaticSorting = false;
    [SerializeField] bool LimitInventorySlots = true;
    [SerializeField] int MaxSlot = 8;
    [SerializeField] ScriptableObjectDatabase ItemDatabse; 

    [Header("Inventory Events:")]
    public InventorySignatureEvent EVT_OnItemUsed = new InventorySignatureEvent();          //Called upon Item usage -------
    public InventoryModifySignatureEvent EVT_OnItemAdded = new InventoryModifySignatureEvent();         //Called upon Item Added -------
    public InventoryModifySignatureEvent EVT_OnItemRemoved = new InventoryModifySignatureEvent();       //Called upon Item Removal -------

    private List<FInventoryInfo> InventoryCache = new List<FInventoryInfo>();                       //Where we store all Items -------
    public ScriptableObjectDatabase ItemDataBaseCache { get { return ItemDatabse; } }

    public void Awake()
    {
        if(LimitInventorySlots)
            for(int i = 0; i < MaxSlot; i++)
                InventoryCache.Add(new FInventoryInfo(null, 0));
    }
    //Check if ItemReference exist in inventory cache
    public bool ContainsItem(Item ItemReference)
    {
        Assert.IsNotNull(ItemReference, "Inventory Error: Could not find item because Parameter is NULL");

        return InventoryCache.Exists(x => x.ItemData == ItemReference);
    }


    //Gets the item by the Item Reference
    public bool GetItem(out FInventoryInfo OutInfo, Item ItemReference)
    {
        Assert.IsNotNull(ItemReference, "Inventory Error: Could not find item because Parameter is NULL");

        OutInfo = InventoryCache.Find(x => x.ItemData == ItemReference);

        return ContainsItem(ItemReference);
    }

    //Gets the item by index of inventory cache
    public bool GetItem(out FInventoryInfo OutInfo, int Index)
    {
        if (Index >= InventoryCache.Count || Index < 0)
        {
            OutInfo = new FInventoryInfo(null, 0);
            return false;
        }

        OutInfo = InventoryCache[Index];
        return true;
    }

    //Check if an item contain on a specific index
    public bool ContainsItemOnIndex(int Index)
    {
        if (Index >= InventoryCache.Count || Index < 0)
            return false;

        return !InventoryCache[Index].IsEmpty();
    }

    //Get total Number of items with reference of that exist in the inventory cache
    public int GetNumberOfItemsOf(Item ItemReference)
    {
        int Total = 0;

        foreach(FInventoryInfo Info in InventoryCache)
        {
            if (Info.ItemData != ItemReference)
                continue;

            Total += Info.Count;
        }

        return Total;
    }

    //Get Total Existing Items
    public int GetNumberOfExistingItems()
    {
        int Total = 0;

        foreach (FInventoryInfo Info in InventoryCache)
        {
            if (Info.IsEmpty())
                continue;

            Total += Info.Count;
        }

        return Total;
    }

    //Check if we can add atleast 1 item to our inventory
    public bool CanAddItem(Item ItemReference)
    {
        if (!LimitInventorySlots)
            return true;

        return InventoryCache.Exists(x => x.ItemData == ItemReference 
                && !x.IsMaxedOut() || x.IsEmpty());
    }

    //Use item on index.
    //------------------------
    public bool UseItemOnIndex(int Index,bool AutomaticRemove = true, int UseCount = 1, GameObject ContextObject = null)
    {
        if (Index >= InventoryCache.Count || Index < 0 || InventoryCache[Index].IsEmpty())
            return false;

        FInventoryInfo UseInfo = InventoryCache[Index];

        if (!UseInfo.ItemData.IsUsable)
            return false;

        bool usedItem = false;

        for(int i = 0; i < UseCount; i++)
        {
            bool usedCurrentItem = UseInfo.ItemData.UseItem(this, ContextObject);

            if (usedCurrentItem && !usedItem)
                usedItem = true;

            if(AutomaticRemove)
                UseInfo.RemoveItem(1);
        }

        EVT_OnItemUsed.Invoke(this, UseInfo.ItemData);
        return usedItem;
    }

    public bool UseItemByReference(Item ItemReference, bool AutomaticRemove = true, int UseCount = 1, GameObject ContextObject = null)
    {
        if (!ContainsItem(ItemReference))
            return false;

        int count = 0;
        bool usedItem = false;

        foreach(FInventoryInfo info in InventoryCache)
        {
            if (info.ItemData != ItemReference)
                continue;

            bool usedCurrentItem = info.ItemData.UseItem(this, ContextObject);
            count++;

            if (usedCurrentItem && !usedItem)
                usedItem = true;

            if (UseCount <= count)
                break;
        }

        EVT_OnItemUsed.Invoke(this, ItemReference);
        return usedItem;
    }

    //Add Item to the Inventory Cache
    //-------------------------------------------
    //Returns:
    //True ---- If successfully added item.
    //False ---- if it failed to add item.
    public bool AddItem(Item ItemReference, out int ItemRemaining, int ItemCount = 1)
    {
        Assert.IsNotNull(ItemReference, "Inventory Error: Could not add item because Item is NULL");

        ItemRemaining = ItemCount;

        if (!CanAddItem(ItemReference) && LimitInventorySlots)
            return false;

        if (!(ItemReference.IsStackable && AddToAvailableSlot(ItemReference, ItemCount, out ItemRemaining)))
            AddToEmptySlot(ItemReference, ItemCount, out ItemRemaining);

        EVT_OnItemAdded.Invoke(this, ItemReference, ItemRemaining);
        return true;
    }
    public bool AddItem(string itemName, out int ItemRemaining, int ItemCount = 1)
    {
        Item ItemReference = ItemDatabse.GetScriptableObject(itemName) as Item;
        Assert.IsNotNull(ItemReference, "Inventory Error: Could not add item because Item is NULL");

        ItemRemaining = ItemCount;

        if (!CanAddItem(ItemReference) && LimitInventorySlots)
            return false;

        if (!(ItemReference.IsStackable && AddToAvailableSlot(ItemReference, ItemCount, out ItemRemaining)))
            AddToEmptySlot(ItemReference, ItemCount, out ItemRemaining);

        EVT_OnItemAdded.Invoke(this, ItemReference, ItemRemaining);
        return true;
    }

    public bool AddItemOnIndex(Item ItemReference, int ItemCount, int Index)
    {
        Assert.IsNotNull(ItemReference, "Inventory Error: Could not add item because Item is NULL");

        if(Index >= InventoryCache.Count || Index < 0)
        {
            Debug.LogError("Index Out of bounds - Could not add Item");
            return false;
        }

        FInventoryInfo info = InventoryCache[Index];

        if(info.IsEmpty())
        {
            int ItemCountAdd = ItemReference.MaxStack < ItemCount ? ItemReference.MaxStack : ItemCount;
            info.SetValue(ItemReference, ItemCountAdd);

            EVT_OnItemAdded.Invoke(this, ItemReference, ItemCountAdd);

            return true;
        }

        return false;
    }

    //Remove Item on Index of
    //-------------------------------------------
    //Returns:
    //True ---- If successfully Removed item.
    //False ---- if it failed to Remove item.
    public bool RemoveItemOnIndex(Item ItemReference, int Index, int ItemCount = 1)
    {
        Assert.IsNotNull(ItemReference, "Inventory Error: Could not Removed item because Item is NULL");

        if (Index >= InventoryCache.Count 
            || Index < 0 || InventoryCache[Index].IsEmpty())
            return false;

        Assert.IsTrue(ItemCount > 0, "Inventory Error: You are removing count that are less than 0! Shame on you sir.");

        FInventoryInfo Info = InventoryCache[Index];
        Info.RemoveItem(ItemCount);

        if (Info.IsEmpty() && AutomaticSorting)
            SortItems();

        EVT_OnItemRemoved.Invoke(this, ItemReference, ItemCount);
        return true;
    }

    //Indiscriminately removes all items with count of
    //-------------------------------------------
    //Returns:
    //True ---- If successfully Removed item.
    //False ---- if it failed to Remove item.
    public bool RemoveItemsOf(Item ItemReference, int ItemCount = 1)
    {
        Assert.IsNotNull(ItemReference, "Inventory Error: Could not Removed item because Item is NULL");

        if (!ContainsItem(ItemReference))
            return false;

        int ItemRemaining = ItemCount;

        for(int i = 0; i < InventoryCache.Count; i++)
        {
            FInventoryInfo Info = InventoryCache[i];

            if (Info.ItemData != ItemReference)
                continue;

            int RemoveCount = Info.Count > ItemRemaining ? ItemRemaining : Info.Count;
            ItemRemaining -= RemoveCount;

            Info.RemoveItem(RemoveCount);

            if (ItemRemaining <= 0)
                break;
        }

        if (AutomaticSorting)
            SortItems();

        EVT_OnItemRemoved.Invoke(this, ItemReference, ItemRemaining);
        return true;
    }

    //Remove all of items with type of
    //-------------------------------------------
    //Returns:
    //True ---- If successfully Removed item.
    //False ---- if it failed to Remove item.
    public bool RemoveAllItemsOf(Item ItemReference)
    {
        Assert.IsNotNull(ItemReference, "Inventory Error: Could not Removed item because Item is NULL");

        if (!ContainsItem(ItemReference))
            return false;

        int NumberOfItem = GetNumberOfItemsOf(ItemReference);

        if(LimitInventorySlots)
        {
            InventoryCache.ForEach(x => 
            {
                if (x.ItemData == ItemReference)
                    x.EmptyInfo();
            });
        }
        else
            InventoryCache.RemoveAll(x => x.ItemData == ItemReference);

        if (AutomaticSorting)
            SortItems();

        EVT_OnItemRemoved.Invoke(this, ItemReference, NumberOfItem);
        return true;
    }

    //Pushes back all empty slots to the back of the list
    public void SortItems()
    {
        if (!LimitInventorySlots)
        {
            InventoryCache.RemoveAll(x => x.IsEmpty());
            return;
        }

        for (int i = 0; i < InventoryCache.Count - 1; i++)
        {
            FInventoryInfo EvaluateInfo = InventoryCache[i];

            if(EvaluateInfo.IsEmpty())
            {
                for(int j = i + 1; j < InventoryCache.Count; j++)
                {
                    FInventoryInfo PrevInfo = InventoryCache[j - 1];
                    FInventoryInfo CurrentInfo = InventoryCache[j];

                    InventoryCache[j] = PrevInfo;
                    InventoryCache[j - 1] = CurrentInfo;
                }
            }
        }
    }


    private bool AddToAvailableSlot(Item ItemReference, int ItemCount, out int Remaining)
    {
        Assert.IsNotNull(ItemReference, "Inventory Error: Could not add item because Item is NULL");

        int CurrentCount = ItemCount;

        foreach (FInventoryInfo Info in InventoryCache)
        {
            if (Info.IsEmpty())
                continue;

            if (Info.ItemData == ItemReference && !Info.IsMaxedOut())
            {
                int RequestAddCount = Info.ItemData.MaxStack - Info.Count;
                int AddCount = RequestAddCount > CurrentCount ?
                    CurrentCount :
                    RequestAddCount;

                Info.Count += AddCount;
                CurrentCount -= AddCount;

                if (CurrentCount <= 0)
                    break;
            }
        }

        Remaining = CurrentCount;
        return CurrentCount <= 0;
    }


    private void AddToEmptySlot(Item ItemReference, int ItemCount, out int CountRemaining)
    {
        Assert.IsNotNull(ItemReference, "Inventory Error: Could not add item because Item is NULL");

        CountRemaining = ItemCount;

        foreach (FInventoryInfo Info in InventoryCache)
        {
            if (CountRemaining <= 0)
                return;

            if (Info.IsEmpty() && ItemReference.IsStackable)
            {
                int count = CountRemaining <= ItemReference.MaxStack ?
                    CountRemaining :
                    ItemReference.MaxStack;

                CountRemaining -= count;

                Info.SetValue(ItemReference, count);
            }
            else if(Info.IsEmpty() && !ItemReference.IsStackable)
            {
                --CountRemaining;
                Info.SetValue(ItemReference, 1);
            }
        }

        if (!LimitInventorySlots)
        {
            while (CountRemaining > 0)
            {
                int count;

                if (ItemReference.IsStackable)
                    count = CountRemaining <= ItemReference.MaxStack ?
                           CountRemaining :
                           ItemReference.MaxStack;
                else
                    count = 1;

                CountRemaining -= count;

                InventoryCache.Add(new FInventoryInfo(ItemReference, count));
            }
        }
    }

    public ESaveType GetSaveType()
    {
        return ESaveType.Persistent;
    }

    public JSONObject SaveData()
    {
        JSONObject inventoryData = new JSONObject();
        JSONArray itemCache = new JSONArray();

        foreach(FInventoryInfo info in InventoryCache)
        {
            JSONObject obj = new JSONObject();

            if (info.IsEmpty())
            {
                obj.Add("Item Name", "None");
                obj.Add("Item Count", 0);
            }
            else
            {
                obj.Add("Item Name", info.ItemData.name);
                obj.Add("Item Count", info.Count);
            }

            itemCache.Add(obj);
        }

        inventoryData.Add("Items", itemCache);

        return inventoryData;
    }

    public void LoadData(JSONObject Data)
    {
        JSONArray arr = Data["Items"].AsArray;

        for(int i = 0; i < arr.Count; i++)
        {
            JSONObject obj = arr[i].AsObject;
            string ItemName = obj["Item Name"];

            if (ItemName == "None")
                continue;

            Item itemPrefab = ItemDatabse.GetScriptableObject(ItemName) as Item;
            AddItemOnIndex(itemPrefab, obj["Item Count"], i);
        }
    }
}

[System.Serializable]
public class InventoryInfoData
{
    public string ItemName;
    public int Count;

    public InventoryInfoData(string Name, int number)
    {
        ItemName = Name;
        Count = number;
    }
}

[System.Serializable]
public class FInventoryInfo
{
    public Item ItemData;
    public int Count;

    public FInventoryInfo(Item p_ItemData, int p_Count)
    {
        ItemData = p_ItemData;
        Count = p_Count;
    }

    public void RemoveItem(int p_Count)
    {
        Count -= p_Count;

        if (Count <= 0)
            EmptyInfo();
    }

    public bool IsEmpty()
    {
        return ItemData == null || Count <= 0;
    }

    public bool CanAddItems(int ItemCount)
    {
        return Count + ItemCount <= ItemData.MaxStack;
    }

    public bool IsMaxedOut()
    {
        if (ItemData == null)
            return false;

        return Count >= ItemData.MaxStack;
    }

    public void SetValue(Item p_ItemData, int p_Count)
    {
        ItemData = p_ItemData;
        Count = p_Count;
    }

    public void EmptyInfo()
    {
        ItemData = null;
        Count = 0;
    }

    public static FInventoryInfo operator+(FInventoryInfo a, int Count)
    {
        return new FInventoryInfo(a.ItemData, a.Count + Count);
    }

    public static FInventoryInfo operator-(FInventoryInfo a, int Count)
    {
        return new FInventoryInfo(a.ItemData, a.Count - Count);
    }
}