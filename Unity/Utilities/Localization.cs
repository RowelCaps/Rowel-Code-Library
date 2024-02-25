using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum Language
{
    English,
    None,
}

public class Localization
{
    public static Language language = Language.English;

    public static Dictionary<string, string> enLocalized;
    public static Dictionary<string, string> noneLocalized;

    public static bool hasInit;

    public static void Init()
    {
        if (hasInit)
            return;

        CSVLoader csvLoader = new CSVLoader();
        csvLoader.LoadCSV();

        enLocalized = csvLoader.GetDictionaryValues("EN");

        hasInit = true;
    }

    public static string GetLocalized(string keyID)
    {
        if (!hasInit)
            Init();
        string value = keyID;

        switch (language)
        {
            case Language.English:  enLocalized.TryGetValue(keyID, out value);  break;
            case Language.None:                                                 break;
        }
        if(value != null)
        {
            value = value.Replace("\\n", "<br>");
            value = value.Replace("/n", "<br>");
            value = value.Replace("@", ",");
        }

        return value != null ? value : keyID;
    }


    public static string GetLocalized(string keyID, string strID1)
    {
        string tempStr = GetLocalized(keyID);
        return tempStr.Replace("%s1", GetLocalized(strID1));
    }
    public static string GetLocalized(string keyID, string strID1, string strID2)
    {
        string tempStr = GetLocalized(keyID, strID1);
        return tempStr.Replace("%s2", GetLocalized(strID2)); ;
    }
    public static string GetLocalized(string keyID, string strID1, string strID2, string strID3)
    {
        string tempStr = GetLocalized(keyID, strID1, strID2);
        return tempStr.Replace("%s3", GetLocalized(strID3)); ;
    }

    public static string GetLocalized(string keyID, float param1)
    {
        string tempStr = GetLocalized(keyID);
        return tempStr.Replace("%s1", param1 + "");
    }
    public static string GetLocalized(string keyID, float param1, float param2)
    {
        string tempStr = GetLocalized(keyID, param1);
        return tempStr.Replace("%s2", param2 + ""); ;
    }
    public static string GetLocalized(string keyID, float param1, float param2, float param3)
    {
        string tempStr = GetLocalized(keyID, param1, param2);
        return tempStr.Replace("%s3", param3 + ""); ;
    }

    public static string GetLocalized(string keyID, int param1)
    {
        string tempStr = GetLocalized(keyID);
        return tempStr.Replace("%s1", param1 + "");
    }
    public static string GetLocalized(string keyID, int param1, int param2)
    {
        string tempStr = GetLocalized(keyID, param1);
        return tempStr.Replace("%s2", param2 + ""); ;
    }
    public static string GetLocalized(string keyID, int param1, int param2, int param3)
    {
        string tempStr = GetLocalized(keyID, param1, param2);
        return tempStr.Replace("%s3", param3 + ""); ;
    }
}
