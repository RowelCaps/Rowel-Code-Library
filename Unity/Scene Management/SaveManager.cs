using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;
using System;
using SimpleJSON;

public enum ESaveType
{
    Scene,
    Persistent
}

public interface ISaveableComponent
{
    ESaveType GetSaveType();
    JSONObject SaveData();
    void LoadData(JSONObject Data);
}

public static class SaveManager
{
    public static string GameDirectory = Application.persistentDataPath + "/Saved";
    public static string GameFullPath = Application.persistentDataPath + "/Saved/Game.txt";
    public static string TempFolderPath = Application.persistentDataPath + "/Temp";
    public static string SceneDataPath = TempFolderPath + "/Level";
    public static string PersistentDataPath = TempFolderPath + "/Persistent";

    public static bool ContainsPersistentObject(string ObjectName)
    {
        if (!Directory.Exists(PersistentDataPath))
            return false;

        string fullpath = PersistentDataPath + "/" + ObjectName + ".txt";
        return File.Exists(fullpath);
    }

    public static bool ContainsSceneData(string SceneName)
    {
        if (!Directory.Exists(SceneDataPath))
            return false;

        string fullpath = SceneDataPath + "/" + SceneName + ".txt";
        return File.Exists(fullpath);
    }

    public static bool ContainsGameData()
    {
        if (!Directory.Exists(GameDirectory))
            return false;

        return File.Exists(GameFullPath);
    }

    private static void SaveToPath(string FolderPath, string FileName, JSONObject Data)
    {
        if (!Directory.Exists(FolderPath))
            Directory.CreateDirectory(FolderPath);

        string FullPath = FolderPath + "/" + FileName + ".txt";
        File.WriteAllText(FullPath, Data.ToString());
    }

    public static void SaveScene(string SceneName, JSONObject Data)
    {
        SaveToPath(SceneDataPath, SceneName, Data);
    }

    public static void SavePersistentObject(string ObjectName, JSONObject Data)
    {
        SaveToPath(PersistentDataPath, ObjectName, Data);
    }
    public static void SaveGame()
    {
        if (!Directory.Exists(GameDirectory))
            Directory.CreateDirectory(GameDirectory);

        JSONObject GameFile = new JSONObject(), 
            SceneData = LoadAllFilesFromPath(SceneDataPath), 
            PersistentData= LoadAllFilesFromPath(PersistentDataPath);

        GameFile.Add("Scene", SceneData);
        GameFile.Add("Persistent", PersistentData);

        File.WriteAllText(GameFullPath, GameFile.ToString());
    }

    public static JSONObject LoadScene(string SceneName)
    {
        string FullPath = SceneDataPath + "/" + SceneName + ".txt";

        if (!File.Exists(FullPath))
            return null;

        string jsonString = File.ReadAllText(FullPath);
        JSONObject sceneJSON = (JSONObject)JSON.Parse(jsonString);

        return sceneJSON;
    }

    public static bool LoadPersistentObject(out JSONObject outJSON, string ObjectName)
    {
        string FullPath = PersistentDataPath + "/" + ObjectName + ".txt";

        if (!File.Exists(FullPath))
        {
            outJSON = null;
            return false;
        }

        string jsonString = File.ReadAllText(FullPath);
        outJSON = (JSONObject)JSON.Parse(jsonString);

        return true;
    }


    public static bool GenerateFilesFromSaveGame()
    {
        if (!File.Exists(GameFullPath))
        {
            Debug.LogError("Cannot generate Save Game");
            return false;
        }

        string jsonString = File.ReadAllText(GameFullPath);

        JSONObject gameJSON = (JSONObject)JSON.Parse(jsonString);

        JSONObject sceneJSON = (JSONObject)JSON.Parse(gameJSON["Scene"].ToString());
        JSONObject persistentJSON = (JSONObject) JSON.Parse(gameJSON["Persistent"].ToString());

        foreach(KeyValuePair<string,JSONNode> kvp in sceneJSON)
        {
            JSONObject SceneObj = (JSONObject)JSON.Parse(sceneJSON[kvp.Key].ToString());

            SaveScene(kvp.Key, SceneObj);
        }

        foreach (KeyValuePair<string, JSONNode> kvp in persistentJSON)
        {
            JSONObject persistentObj = (JSONObject)JSON.Parse(persistentJSON[kvp.Key].ToString());
            SavePersistentObject(kvp.Key, persistentObj);
        }

        return true;
    }

    public static void DeleteTemporaryFiles()
    {
        DeletePath(SceneDataPath);
        DeletePath(PersistentDataPath);
        DeletePath(TempFolderPath);
    }

    public static void DeleteGameFile()
    {
        if (!File.Exists(GameFullPath))
            return;

        File.Delete(GameFullPath);
    }

    private static void DeletePath(string Path)
    {
        if (!Directory.Exists(Path))
            return;

        string[] FilesFound = Directory.GetFiles(Path);

        for(int i = 0; i < FilesFound.Length; i ++)
        {
            File.Delete(FilesFound[i]);
        }

        Directory.Delete(Path);
    }

    private static JSONObject LoadAllFilesFromPath(string Path)
    {
        JSONObject fileData = new JSONObject();

        if (!Directory.Exists(Path))
        {
            fileData.Add("NULL", true);
            return fileData;
        }

        DirectoryInfo info = new DirectoryInfo(Path);
        FileInfo[] files = info.GetFiles("*.txt");

        foreach (FileInfo file in files)
        {
            if (!file.Exists)
                continue;

            string FullPath = Path + "/" + file.Name;
            string jsonString = File.ReadAllText(FullPath);
            JSONObject sceneJSON = (JSONObject)JSON.Parse(jsonString);

            string ID = System.IO.Path.GetFileNameWithoutExtension(file.Name); ;
            fileData.Add(ID, sceneJSON);
        }

        return fileData;
    }
}


