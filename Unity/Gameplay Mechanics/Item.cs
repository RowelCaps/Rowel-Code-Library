using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[System.Serializable]
[CreateAssetMenu(menuName = "Item/Default Item", fileName = "Item.asset")]
public class Item : ScriptableObject
{
    [Header("Item Data")]
    [SerializeField]
    protected string itemName = "None";                     //Name of the Item  ------

    [SerializeField]
    protected Sprite itemImage;                             //Sprite for UI use -----

    [SerializeField, TextArea(2, 12)]                       //Description of the item   -------
    protected string itemDescription = "Hello World";

    [Header("Item Settings")]
    [SerializeField] protected GameObject ObjectPrefab;
    [SerializeField] protected bool isUsable = true;        //Check if we can use it on inventry -------
    [SerializeField] protected bool isStackable = false;    //Is the item stackable for inventory -------
    [SerializeField] protected int maxStack = 1;            //Max stacks for inventory ------
    [SerializeField] protected bool isDiscardable = true;


    public string ItemName { get { return itemName; } }
    public Sprite ItemImage { get { return itemImage; } }
    public bool IsUsable { get { return isUsable; } }
    public bool IsStackable { get { return isStackable; } }
    public int MaxStack { get { return maxStack; } }
    public bool IsDiscardable { get { return isDiscardable; } }

    public string ItemDescription { get { return itemDescription; } }

    //Overridable function
    //---------------------------------
    //Override these 3 functions if you want to add functionality to your item
    public virtual bool UseItem(Inventory Owner, GameObject ContextObject = null) { return false; }
    public virtual void OnItemAdded(Inventory Owner) { }
    public virtual void OnItemRemoved(Inventory Owner) { }

#if UNITY_EDITOR
    //Make sure when user unchecks IsStackable set maxstack to 1
    private void OnValidate()
    {
        if (!isStackable)
            maxStack = 1;
    }
#endif
}
