using System.Collections;
using System.Collections.Generic;
using System;
using UnityEngine;
using SimpleJSON;

public abstract class Quest : MonoBehaviour
{
    [Header("Quest Settings")]
    [SerializeField] protected string questName = "None";
    [SerializeField, TextArea(2, 12)] 
    protected string questDescription = "Sam paki send saakin link ng leaks ni belle delphine";

    public string QuestName { get { return questName; } }
    public string QuestDescription { get { return questDescription; } }

    //Calls upon when quest initially added
    public virtual void OnQuestBegin(QuestReceiver Owner) { }

    //Calls upon when quest is about to be end
    public virtual void OnQuestEnd(QuestReceiver Owner) { }

    //This is used to check whether we should finish the quest or progress it
    //----------------------------------------
    //Example - I would like to check if the inventory contains number of items? if not then Owner.FinishQuest(this);
    public abstract void EvaluateQuest(QuestReceiver Owner, GameObject Instigator);

    //public virtual SaveComponentData SaveData() { return new SaveComponentData(); }

    //public virtual void LoadData(SaveComponentData SaveData) { }

    public virtual JSONObject SaveData() 
    {
        JSONObject data = new JSONObject();
        return data;
    }
    public virtual void LoadData(JSONObject Data) { }

    //Override Operators so we could check things easily
    public static bool operator==(Quest a, Quest b)
    {
        if(object.ReferenceEquals(a, null))
        {
            return object.ReferenceEquals(b, null);
        }

        return a.Equals(b);
    }
    public static bool operator !=(Quest a, Quest b)
    {
        if (object.ReferenceEquals(a, null))
        {
            return !object.ReferenceEquals(b, null);
        }

        return !a.Equals(b);
    }

    public override bool Equals(object other)
    {
        Quest b = other as Quest;

        if (b == null)
            return false;

        return QuestName == b.QuestName && QuestDescription == b.QuestDescription;
    }

    public override int GetHashCode()
    {
        return base.GetHashCode();
    }

    //public ESaveType GetSaveType()
    //{
    //    return ESaveType.Persistent;
    //}
}
