using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.Assertions;
using System.Linq;

public static class SingletonManager
{
    private static Dictionary<System.Type, Component> singletonCache = new Dictionary<System.Type, Component>();

    static SingletonManager()
    {
        SceneManager.sceneUnloaded += ClearCache;
    }

    //be careful registering object, once you already register a component, any component with the same type will be destroyed if registered
    public static void RegisterComponent<T>(T singletonObject) where T : Component
    {
        Assert.IsFalse(singletonCache.ContainsKey(singletonObject.GetType()));

        if (singletonCache.ContainsKey(singletonObject.GetType()) && singletonCache[singletonObject.GetType()] != singletonObject)
        {
            Object.Destroy(singletonObject.gameObject);
            return;
        }

        singletonCache.Add(typeof(T), singletonObject);
    }

    //Removes a component from the Singleton Cache
    public static void UnregisterComponent<T>() where T : Component
    {
        if (!singletonCache.ContainsKey(typeof(T)))
            return;

        singletonCache.Remove(typeof(T));
    }

    //use this function like GetComponent<>(); SingletonManager.GetSingleton<SomeShit>();
    //This is to get singleton object ur after, please make sure u registered it.
    public static T GetSingleton<T>() where T : Component
    {
        Assert.IsTrue(singletonCache.ContainsKey(typeof(T)));

        if (!singletonCache.ContainsKey(typeof(T)))
            return null;

        return singletonCache[typeof(T)] as T;

    }

    //Check if singleton cache contains key
    public static bool HasKey(System.Type type)
    {
        return singletonCache.ContainsKey(type);
    }

    //Remove all cache from a certain scene
    private static void ClearCache(Scene current)
    {
        Dictionary<System.Type, Component> temp = singletonCache.ToDictionary(entry => entry.Key, entry => entry.Value);

        foreach (KeyValuePair<System.Type, Component> singleton in temp)
        {
            if (singleton.Value == null)
                singletonCache.Remove(singleton.Key);
        }
    }
}
