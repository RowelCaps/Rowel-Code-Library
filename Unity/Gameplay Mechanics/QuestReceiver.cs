using SimpleJSON;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Events;

[System.Serializable]
public class QuestSignatureEvent : UnityEvent<Quest> { }

public class QuestReceiver : MonoBehaviour, ISaveableComponent
{
    [Header("Quest Receiver Settings")]
    [SerializeField] private PrefabDatabase QuestDatabase;

    [Header("Quest Receiver Events")]
    public QuestSignatureEvent EVT_OnQuestAdded;
    public QuestSignatureEvent EVT_OnQuestEvaluation;
    public QuestSignatureEvent EVT_OnQuestFinished;

    List<Quest> currentQuests = new List<Quest>();
    List<Quest> finishedQuests = new List<Quest>();

    public List<Quest> CurrentQuest { get { return currentQuests; } }
    public List<Quest> FinishedQuests { get { return finishedQuests; } }

    public PrefabDatabase QuestDatabaseCache { get { return QuestDatabase; } }

    //Check if the QuestReference is on going
    public bool IsQuestActive(Quest QuestReference)
    {
        return currentQuests.Contains(QuestReference);
    }

    //Check if the QuestReference is Finished
    public bool IsQuestFinished(Quest QuestReference)
    {
        return FinishedQuests.Contains(QuestReference);
    }

    public Quest GetCurrentQuest(int Index)
    {
        if (currentQuests[0] == null)
        {
            Debug.Log("Quest on index " + Index + " is NULL");
            return null;
        }

        return QuestDatabase.GetPrefabByName(currentQuests[Index].gameObject.GetPrefabName()).GetComponent<Quest>();
    }

    //Tries to register quest to the cache
    //-----------------------------------------
    //Return value:
    //true ----- If Registered quest is success
    //false ---- Something went wrong and we could not add the quest
    public bool RegisterQuest(Quest QuestReference)
    {
        Assert.IsNotNull(QuestReference, "QuestReceiver Error: You are trying to Register a NULL quest");

        if (currentQuests.Contains(QuestReference) || FinishedQuests.Contains(QuestReference))
            return false;

        Quest questInstance = Instantiate(QuestReference);
        questInstance.OnQuestBegin(this);
        questInstance.transform.SetParent(transform);

        currentQuests.Add(questInstance);
        EVT_OnQuestAdded.Invoke(questInstance);
        return true;
    }
    public bool RegisterQuest(string QuestName)
    {
        Quest QuestReference = QuestDatabase.GetPrefabByName(QuestName).GetComponent<Quest>();

        Assert.IsNotNull(QuestReference, "QuestReceiver Error: You are trying to Register a NULL quest");

        if (currentQuests.Contains(QuestReference) || FinishedQuests.Contains(QuestReference))
            return false;

        Quest questInstance = Instantiate(QuestReference);
        questInstance.OnQuestBegin(this);
        questInstance.transform.SetParent(transform);

        currentQuests.Add(questInstance);
        EVT_OnQuestAdded.Invoke(questInstance);
        return true;
    }

    //Fire the event evaluation and evaluate the requested Quest
    public void RequestQuestEvaluation(Quest QuestReference, GameObject Instigator = null)
    {
        if (QuestReference == null )
            return;

        Assert.IsNotNull(QuestReference, "QuestReceiver Error: You are trying to Evaluate a NULL quest");

        if (!currentQuests.Contains(QuestReference))
            return;

        Quest quest = currentQuests.Find(x => x == QuestReference);
        quest.EvaluateQuest(this, Instigator);

        EVT_OnQuestEvaluation.Invoke(quest);
    }

    //End quest and stores it to the end quest cache
    public void EndQuest(Quest QuestReference)
    {
        Assert.IsNotNull(QuestReference, "QuestReceiver Error: You are trying to End a NULL quest");

        if (!currentQuests.Contains(QuestReference))
            return;

        Quest quest = currentQuests.Find(x => x == QuestReference);

        EVT_OnQuestFinished.Invoke(quest);

        Assert.IsNotNull(QuestDatabase, "No quest database cannot be found");
        GameObject questPrefab = QuestDatabase.GetPrefabByName(quest.name);

        Assert.IsNotNull(questPrefab, "Finished quest: Error Catch - questPrefab could not be seen");

        currentQuests.Remove(quest);
        FinishedQuests.Add(questPrefab.GetComponent<Quest>());

        quest.OnQuestEnd(this);
    }

    public ESaveType GetSaveType()
    {
        return ESaveType.Persistent;
    }

    public JSONObject SaveData()
    {
        JSONObject obj = new JSONObject();
        JSONArray CurrentQuestArray = new JSONArray(),
            FinishedQuestArray = new JSONArray();

        foreach(Quest quest in CurrentQuest)
        {
            JSONObject questData = new JSONObject();

            questData.Add("Quest Name", quest.gameObject.GetPrefabName());
            questData.Add("Quest Saved Data", quest.SaveData());

            CurrentQuestArray.Add(questData);
        }

        FinishedQuests.ForEach(x =>
           {
               FinishedQuestArray.Add(x.gameObject.name);
           });

        obj.Add("CurrentQuest", CurrentQuestArray);
        obj.Add("FinishedQuest", FinishedQuestArray);

        return obj;
    }

    public void LoadData(JSONObject Data)
    {
        JSONArray currentQuestArray = Data["CurrentQuest"].AsArray;
        //Loads Current Quest
        for(int i = 0; i < currentQuestArray.Count; i++)
        {
            JSONObject currentQuestData = currentQuestArray[i].AsObject;

            GameObject questPrefab = QuestDatabase.GetPrefabByName(currentQuestData["Quest Name"]);
            GameObject questInstance = Instantiate(questPrefab, transform);

            Quest quest = questInstance.GetComponent<Quest>();
            quest.LoadData(currentQuestData["Quest Saved Data"].AsObject);

            currentQuests.Add(quest);
        }
        //Load Finished Quest

        JSONArray finishQuestArray = Data["FinishedQuest"].AsArray;

        for (int i = 0; i < finishQuestArray.Count; i++)
        {
            GameObject questPrefab = QuestDatabase.GetPrefabByName(finishQuestArray[i]);
            Quest quest = questPrefab.GetComponent<Quest>();

            FinishedQuests.Add(quest);
        }
    }
}

