using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class GameManager : Singleton<GameManager>
{
    List<Resolution> m_resolutions = new List<Resolution>();
    int currentResolution;

    public List<Resolution> Resolutions { get => m_resolutions; }
    public int CurrentResolution { get => currentResolution; }

    private void Awake()
    {
        Resolution[] resolutions = Screen.resolutions;

        for (int i = 0; i < resolutions.Length; i++)
        {
            Resolution reso = resolutions[i];

            if ((float)reso.width / (float)reso.height >= 1.6f)
            {
                if (Resolutions.FindIndex(x => x.width == reso.width && x.height == reso.height) >= 0)
                    continue;

                Resolutions.Add(reso);
            }
        }

        for (int i = 0; i < m_resolutions.Count; i++)
        {
            if (m_resolutions[i].width == Screen.currentResolution.width && m_resolutions[i].height == Screen.currentResolution.height)
            {
                currentResolution = i;
            }
        }

        if (currentResolution > 0)
            currentResolution -= 1;

        Resolution res = m_resolutions[currentResolution];
        Screen.SetResolution(res.width, res.height, FullScreenMode.Windowed);

        InitStartScenes();
    }

    public void InitStartScenes()
    {
        List<string> startScene = new List<string>() { "UI" };

        StartCoroutine(LoadMultipleSceneAsync(startScene, x =>
        {
            if (x == "UI")
            {
                UIManager.Instance.Init();

                BaseTransitionCanvas transitionCanvas = UIManager.Instance.GetTransitionCanvas("BlackScreenCanvas");
                transitionCanvas.gameObject.SetActive(true);

                transitionCanvas.BaseCanvasGroup.alpha = 1;

                UIManager.Instance.OpenCanvas("MainMenuCanvas", false);

                MainMenuCanvas mainMenuCanvas = UIManager.Instance.GetCanvas<MainMenuCanvas>();

                mainMenuCanvas.BaseCanvasGroup.interactable = false;
                mainMenuCanvas.BaseCanvasGroup.blocksRaycasts = false;

                transitionCanvas.StartTransitionSingle(0, 1f, () =>
                {
                    mainMenuCanvas.BaseCanvasGroup.interactable = true;
                    mainMenuCanvas.BaseCanvasGroup.blocksRaycasts = true;
                });
            }
        }));
    }

    public void LoadSceneAsync(string SceneName, System.Action OnFinishCallback = null)
    {
        StartCoroutine(LoadSceneAsynchronously(SceneName, OnFinishCallback));
    }

    public void UnloadSceneAsync(string SceneName, System.Action OnFinishCallback = null)
    {
        StartCoroutine(UnloadSceneAsynchronously(SceneName, OnFinishCallback));
    }


    private IEnumerator LoadMultipleSceneAsync(List<string> sceneOrder, System.Action<string> onFinishLoadingScene = null)
    {
        foreach(string scene in sceneOrder)
        {
            yield return LoadSceneAsynchronously(scene, () =>
            {
                if (onFinishLoadingScene != null)
                    onFinishLoadingScene.Invoke(scene);
            });

            yield return null;
        }
    }

    private IEnumerator LoadSceneAsynchronously(string sceneName, System.Action onFinishLoadingAction = null)
    {
        AsyncOperation operation = SceneManager.LoadSceneAsync(sceneName, LoadSceneMode.Additive);
        
        while(!operation.isDone)
        {
            float progress = Mathf.Clamp01(operation.progress / 0.9f);

            yield return null;
        }

        yield return null;

        if (onFinishLoadingAction != null)
            onFinishLoadingAction.Invoke();
    }

    private IEnumerator UnloadSceneAsynchronously(string sceneName, System.Action onFinishUnloadingAction = null)
    {
        Scene scene = SceneManager.GetSceneByName(sceneName);
        AsyncOperation operation = SceneManager.UnloadSceneAsync(scene.buildIndex);

        while (!operation.isDone)
        {
            float progress = Mathf.Clamp01(operation.progress / 0.9f);

            yield return null;
        }

        yield return null;

        if (onFinishUnloadingAction != null)
            onFinishUnloadingAction.Invoke();
    }
}
