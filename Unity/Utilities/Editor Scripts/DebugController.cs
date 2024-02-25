using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;
using UnityEngine.EventSystems;
using UnityEngine.SceneManagement;
using System;
using System.Linq;

public class DebugController : MonoBehaviour
{
    private bool ShowDebugCommand = false;
    private string InputCommand = "";

    Dictionary<string, object> commandLookUp = new Dictionary<string, object>();

    private bool containsConsoleMessage = false;
    private bool showHelpMenu = false;

    private string consoleMessage;

    private void Awake()
    {
        SingletonManager.RegisterComponent(this);

        RegisterCommand("ShowHelpMenu","ShowHelpMenu", "Display help menu in the debug console", () => 
        { 
            showHelpMenu = true; 
        });

        RegisterCommand("CloseHelpMenu","CloseHelpMenu", "Close help menu in the debug console", () =>
        {
            showHelpMenu = false;
        }); 
        
        RegisterCommand<int>("SetLevel", "SetLevel <i> level number </i>", "Sets the level index (Starts with 0)", (int i) =>
        {
            LevelManager levelManager = SingletonManager.GetSingleton<LevelManager>();
            levelManager.SetLevel((ELevelState)i);
        });

        RegisterCommand("Clear", "Clear", "Clears up console message", () =>
        {
            containsConsoleMessage = false;
            consoleMessage = "";
        });

        RegisterCommand<string, int>("Player_AddItem", "Player_AddItem '<i>ItemName' 'ItemCount'</i>", "Adds Item to player Inventory", (string ItemName, int Count) =>
        {
            GameObject player = GameObject.FindGameObjectWithTag("Player");

            if (player == null)
            {
                containsConsoleMessage = true;
                consoleMessage = "No player found in loaded scenes";

                return;
            }

            Inventory inventory = player.GetComponent<Inventory>();
            int remainingItems = 0;

            if(!inventory.ItemDataBaseCache.DoesScriptableExist(ItemName))
            {
                containsConsoleMessage = true;
                consoleMessage = ItemName = " Item does not exist in Item DataBase!";

                return;
            }

            if(!inventory.AddItem(ItemName, out remainingItems, Count))
            {
                containsConsoleMessage = true;
                consoleMessage = "Could not add " + ItemName + "in Player Inventory";
            }
        });

        RegisterCommand("Kill_All_AI", "Kill_All_AI", "Deactivates All AI in the scene", () =>
        {
            AIController[] controller = Resources.FindObjectsOfTypeAll<AIController>();

            foreach(AIController ai in controller)
            {
                if (ai.gameObject.scene.name == null)
                    return;

                ai.gameObject.SetActive(false);
            }
        });

        RegisterCommand("IAmRikimaru", "IAmRikimaru", "Makes Player Invisible to all enemy AI", () =>
        {
            GameObject player = GameObject.FindGameObjectWithTag("Player");

            if (player == null)
            {
                containsConsoleMessage = true;
                consoleMessage = "No player found in loaded scenes";

                return;
            }

            player.layer = LayerMask.NameToLayer("Default");
        });

        RegisterCommand("GemOfTrueSight", "GemOfTrueSight", "Makes Player visible to all enemy AI", () =>
        {
            GameObject player = GameObject.FindGameObjectWithTag("Player");

            if (player == null)
            {
                containsConsoleMessage = true;
                consoleMessage = "No player found in loaded scenes";

                return;
            }

            player.layer = LayerMask.NameToLayer("Player");
        });

        RegisterCommand<string>("LoadScene", "LoadScene <i>'Scene'</i>", "Jump to specified scene", (string SceneName) =>
        {
            if(SceneUtility.GetBuildIndexByScenePath(SceneName) < 0)
            {
                containsConsoleMessage = true;
                consoleMessage = SceneName + " is not on the build index!";

                return;
            }

            PersistentSceneManager sceneManager = SingletonManager.GetSingleton<PersistentSceneManager>();
            sceneManager.TransitionToNextScene(SceneName, true);
        });

        RegisterCommand("SaveGame", "SaveGame", "Saves the current state of the game", () =>
        {
            PersistentSceneManager sceneManager = SingletonManager.GetSingleton<PersistentSceneManager>();
            sceneManager.SaveGame();
        });

        RegisterCommand("LoadGame", "LoadGame", "Loads game from file", () =>
        {
            PersistentSceneManager sceneManager = SingletonManager.GetSingleton<PersistentSceneManager>();
            sceneManager.LoadGame();
        });

        RegisterCommand("LetThereBeLight", "LetThereBeLight", "increase 100000 oil capacity to the lamp", () =>
        {
            GameObject player = GameObject.FindGameObjectWithTag("Player");

            if (player == null)
            {
                containsConsoleMessage = true;
                consoleMessage = "No player found in loaded scenes";

                return;
            }

            LampComponent lamp = player.GetComponent<LampComponent>();
            lamp.IncreaseOilCapacity(100000, true);
        });

        RegisterCommand<float>("AddOil", "AddOil <i>'Oil Amount'</i>", "Adds oil to the lamp", (float oilCount) =>
        {
            GameObject player = GameObject.FindGameObjectWithTag("Player");

            if (player == null)
            {
                containsConsoleMessage = true;
                consoleMessage = "No player found in loaded scenes";

                return;
            }

            LampComponent lamp = player.GetComponent<LampComponent>();
            lamp.AddOil(oilCount);
        });

        RegisterCommand("MysteryCommand", "MysteryCommand", "Execute a mystery function", () =>
        {
            containsConsoleMessage = true;
            consoleMessage = "Johann = bimby";
        });

        RegisterCommand("RuneOfHaste", "RuneOfHaste", "Increses player walk speed", () =>
        {
            GameObject player = GameObject.FindGameObjectWithTag("Player");

            if (player == null)
            {
                containsConsoleMessage = true;
                consoleMessage = "No player found in loaded scenes";

                return;
            }

            PlayerMovement movement = player.GetComponent<PlayerMovement>();
            movement.WalkSpeed = 20;
        });

        RegisterCommand<string>("AddQuest", "AddQuest <i>'QuestName'</i>", "Adds Quest to player", (string QuestName) =>
        {
            GameObject player = GameObject.FindGameObjectWithTag("Player");

            if (player == null)
            {
                containsConsoleMessage = true;
                consoleMessage = "No player found in loaded scenes";

                return;
            }

            QuestReceiver questReceiver = player.GetComponent<QuestReceiver>();

            if(!questReceiver.RegisterQuest(QuestName))
            {
                consoleMessage = "Failed to add Quest";
                containsConsoleMessage = true;

                return;
            }
        });

        RegisterCommand<string>("FinishQuest", "FinishQuest <i>'QuestName'</i>", "Finishes the specified quest", (string QuestName) =>
        {
            GameObject player = GameObject.FindGameObjectWithTag("Player");

            if (player == null)
            {
                containsConsoleMessage = true;
                consoleMessage = "No player found in loaded scenes";

                return;
            }

            QuestReceiver questReceiver = player.GetComponent<QuestReceiver>();
            PrefabDatabase database = questReceiver.QuestDatabaseCache;

            if(!database.DoesPrefabExist(QuestName))
            {
                containsConsoleMessage = true;
                consoleMessage = "Invalid Quest!";

                return;
            }

            Quest prefab = database.GetPrefabByName(QuestName).GetComponent<Quest>();
            questReceiver.EndQuest(prefab);
        });

        SortCommandAlphabetically();
    }

    private void Update()
    {
        if(Input.GetKeyDown(KeyCode.BackQuote))
        {
            ShowDebugCommand = !ShowDebugCommand;
        }
    }

    public void RegisterCommand(string ID, string Name, string Description, Action action)
    {
        if (commandLookUp.ContainsKey(Name))
            return;

        DebugCommand debugCommand = new DebugCommand(Name, Description, action);
        commandLookUp.Add(ID, debugCommand);
    }

    public void RegisterCommand<T>(string ID, string Name, string Description, Action<T> action)
    {
        if (commandLookUp.ContainsKey(Name))
            return;

        DebugCommand<T> debugCommand = new DebugCommand<T>(Name, Description, action);
        commandLookUp.Add(ID, debugCommand);
    }
    public void RegisterCommand<T1,T2>(string ID, string Name, string Description, Action<T1,T2> action)
    {
        if (commandLookUp.ContainsKey(Name))
            return;

        DebugCommand<T1, T2> debugCommand = new DebugCommand<T1, T2>(Name, Description, action);
        commandLookUp.Add(ID, debugCommand);
    }

    private void HandleInput()
    {
        string[] properties = InputCommand.Split(' ');
        string commandID = properties[0];

        if (!commandLookUp.ContainsKey(commandID))
        {
            consoleMessage = InputCommand + " is not recognized! Enter command 'ShowHelpMenu' to open the command menu.";
            containsConsoleMessage = true;
            return;
        }

        DebugCommandBase commandBase = commandLookUp[commandID] as DebugCommandBase;

        if(commandBase is DebugCommand command)
        {
            command.Invoke();
        }
        else if(commandBase is DebugCommand<int> commandInt)
        {
            int count = int.Parse(properties[1]);
            commandInt.Invoke(count);
        }
        else if(commandBase is DebugCommand<string, int> commandStringInt)
        {
            int count = int.Parse(properties[2]);
            commandStringInt.Invoke(properties[1], count);
        }
        else if (commandBase is DebugCommand<float> commandfloat)
        {
            float count = float.Parse(properties[1]);
            commandfloat.Invoke(count);
        }
        else if(commandBase is DebugCommand<string> commandString)
        {
            commandString.Invoke(properties[1]);
        }
    }

    private void SortCommandAlphabetically()
    {
        Dictionary<string, object> temp = new Dictionary<string, object>();

        foreach (KeyValuePair<string, object> kvp in commandLookUp.OrderBy(k => k.Key))
        {
            temp.Add(kvp.Key, kvp.Value);
        }

        commandLookUp = temp;
    }

    private void OnGUI()
    {
        if (!ShowDebugCommand)
        {
            InputCommand = "";
            return;
        }

        if (Event.current.Equals(Event.KeyboardEvent("return")))
        {
            HandleInput();
        }

        float y = 0;
        GUI.backgroundColor = new Color(1f, 1f, 1f, 0.4f);

        if (showHelpMenu)
            DrawHelpMenu(ref y);

        if (containsConsoleMessage)
            DrawDebugMessage(ref y);

        string prevInput = InputCommand;

        GUI.Box(new Rect(0f, y, Screen.width, 30), "");
        InputCommand = GUI.TextField(new Rect(10, y + 5, Screen.width, 30), InputCommand);
    }

    private void DrawDebugMessage(ref float y)
    {
        Rect messageRect = new Rect(0, y, Screen.width, Screen.height * 0.05f);
        GUI.Box(messageRect, "");

        Color defaultCol = GUI.color;
        GUI.color = Color.red;

        GUI.Label(new Rect(5, y + messageRect.height * 0.2f, Screen.width, messageRect.height * 0.7f), consoleMessage);
        y += messageRect.height;

        GUI.color = defaultCol;
    }

    Vector2 scroll;

    private void DrawHelpMenu(ref float y)
    {
        Rect scrollBG = new Rect(0, y, Screen.width, Screen.height * 0.2f);
        GUI.Box(scrollBG, "Command Menu");

        float viewportHeight = (Screen.height * 0.04f);
        Rect scrollViewport = new Rect(0, 0, Screen.width * 0.9f, viewportHeight * commandLookUp.Count);

        Rect scrollViewRect = new Rect(0, y + scrollBG.height * 0.1f, Screen.width, scrollBG.height * 0.9f);
        scroll = GUI.BeginScrollView(scrollViewRect, scroll, scrollViewport);

        int index = 0;

        foreach(KeyValuePair<string, object> kvp in commandLookUp)
        {
            DebugCommandBase command = kvp.Value as DebugCommandBase;

            Rect elementRect = new Rect(5, viewportHeight * index, scrollViewport.width - 100, viewportHeight);

            GUI.Label(elementRect, "<color=#43d063> " +  command.Name + "</color>"+ " - " + command.Description);

            ++index;
        }

        GUI.EndScrollView();

        y += scrollBG.height;
    }

    public enum ECommandOutput
    {
        None,
        ShowMenu,
        ShowConsoleMessage
    }
}
