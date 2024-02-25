using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Linq;
    
public class UIManager : Singleton<UIManager>
{
    //Created a dictionary for canvas for faster iteration & Searching
    private Dictionary<string, BaseCanvas> m_UICanvasDictionary = new Dictionary<string, BaseCanvas>();

    private Dictionary<string, BaseTransitionCanvas> m_transitionCanvasDictionary = new Dictionary<string, BaseTransitionCanvas>();

    private List<BaseCanvas> m_openedCanvas = new List<BaseCanvas>();

    public void Init()
    {
        //Find all base canvases. Use Find Object of type since this is only called once
        BaseCanvas[] mainCanvasList = Object.FindObjectsOfType<BaseCanvas>();

        foreach(BaseCanvas canvas in mainCanvasList)
        {
            if (m_UICanvasDictionary.ContainsKey(canvas.CanvasID))
                continue;

            m_UICanvasDictionary.Add(canvas.CanvasID, canvas);

            canvas.BaseCanvasGroup.alpha = 0;
            canvas.BaseCanvasGroup.blocksRaycasts = false;
            canvas.BaseCanvasGroup.interactable = false;

            canvas.gameObject.SetActive(false);
        }

        BaseTransitionCanvas[] transitionCanvas = Object.FindObjectsOfType<BaseTransitionCanvas>();

        foreach (BaseTransitionCanvas transition in transitionCanvas)
        {
            if (m_transitionCanvasDictionary.ContainsKey(transition.TransitionID))
                continue;

            m_transitionCanvasDictionary.Add(transition.TransitionID, transition);

            transition.BaseCanvasGroup.alpha = 0;
            transition.BaseCanvasGroup.blocksRaycasts = false;
            transition.BaseCanvasGroup.interactable = false;
        }
    }

    public BaseTransitionCanvas GetTransitionCanvas(string transitionID)
    {
        if (!m_transitionCanvasDictionary.ContainsKey(transitionID))
            return null;

        return m_transitionCanvasDictionary[transitionID];
    }
    public BaseCanvas GetCanvs(string transitionID)
    {
        if (!m_UICanvasDictionary.ContainsKey(transitionID))
            return null;

        return m_UICanvasDictionary[transitionID];
    }


    /*ID            - Open the canvas with ID
      isAdditive    - Hide all UI that are active
      useTransition - should we use a transition interpolation of the canvas alpha
     finishCallback - Called when canvas is finish opening
    */
    public void OpenCanvas(string ID, bool useTransition = true, System.Action finishCallback = null)
    {
        if(!m_UICanvasDictionary.ContainsKey(ID))
        {
            Debug.LogWarning("Could Not find canvas");

            return;
        }

        BaseCanvas canvas = m_UICanvasDictionary[ID];

        BaseCanvas lastOpenedCanvas = m_openedCanvas.Count > 0 ? m_openedCanvas[m_openedCanvas.Count -1] : null;

        if(lastOpenedCanvas != null)
        {
            lastOpenedCanvas.BaseCanvasGroup.interactable = false;
            lastOpenedCanvas.BaseCanvasGroup.blocksRaycasts = false;
        }

        if (useTransition)
        {
            StartCoroutine(TransitionCanvas(canvas, 1, 1,
                (x) => 
                { 
                    if(lastOpenedCanvas != null)
                    {
                        lastOpenedCanvas.BaseCanvasGroup.alpha = 1 - x;
                    }
                }, 
                () =>
                {
                    canvas.BaseCanvasGroup.interactable = true;
                    canvas.BaseCanvasGroup.blocksRaycasts = true;

                    m_openedCanvas.Add(canvas);

                    if (lastOpenedCanvas != null)
                        lastOpenedCanvas.gameObject.SetActive(false);

                    if(finishCallback != null)
                        finishCallback.Invoke();
                }));

            return;
        }

        if (lastOpenedCanvas != null)
        {
            lastOpenedCanvas.gameObject.SetActive(false);
            lastOpenedCanvas.BaseCanvasGroup.alpha = 1;
        }

        canvas.gameObject.SetActive(true);

        canvas.BaseCanvasGroup.alpha = 1;
        canvas.BaseCanvasGroup.interactable = true;
        canvas.BaseCanvasGroup.blocksRaycasts = true;

        m_openedCanvas.Add(canvas);

        if(finishCallback != null)
            finishCallback.Invoke();

    }

    public void ReturnToPreviousCanvas(bool useTransition = true, System.Action OnFinishCallback = null)
    {
        if (m_openedCanvas.Count <= 0)
            return;

        BaseCanvas lastOpenedCanvas = m_openedCanvas.Count > 0 ? m_openedCanvas[m_openedCanvas.Count -1] : null;
        BaseCanvas inQueueCanvas = m_openedCanvas.Count > 1 ? m_openedCanvas[m_openedCanvas.Count - 2] : null;

        if (!useTransition)
        {
            if(inQueueCanvas != null)
            {
                inQueueCanvas.BaseCanvasGroup.interactable = true;
                inQueueCanvas.BaseCanvasGroup.blocksRaycasts = true;
                inQueueCanvas.gameObject.SetActive(true);
            }

            lastOpenedCanvas.BaseCanvasGroup.interactable = false;
            lastOpenedCanvas.BaseCanvasGroup.blocksRaycasts = false;

            lastOpenedCanvas.gameObject.SetActive(false);

            return;
        }

        if (inQueueCanvas != null)
        {
            inQueueCanvas.gameObject.SetActive(true);

            inQueueCanvas.BaseCanvasGroup.interactable = false;
            inQueueCanvas.BaseCanvasGroup.blocksRaycasts = false;
        }

        StartCoroutine(TransitionCanvas(lastOpenedCanvas, 0, 0.5f,
            x =>
            {
                if (inQueueCanvas != null)
                    inQueueCanvas.BaseCanvasGroup.alpha = x;
            },
            ()=>
            {
                lastOpenedCanvas.BaseCanvasGroup.interactable = false;
                lastOpenedCanvas.BaseCanvasGroup.blocksRaycasts = false;

                lastOpenedCanvas.gameObject.SetActive(false);

                if (inQueueCanvas != null)
                {
                    inQueueCanvas.BaseCanvasGroup.interactable = true;
                    inQueueCanvas.BaseCanvasGroup.blocksRaycasts = true;
                }

                if (OnFinishCallback != null)
                    OnFinishCallback.Invoke();
            }));
    }

    public IEnumerator TransitionCanvas(BaseCanvas canvas, float targetAlpha, float duration,System.Action<float> transitionCallback = null, System.Action finishCallback = null)
    {
        float currentTime = 0;
        float initialAlpha = canvas.BaseCanvasGroup.alpha; 

        canvas.gameObject.SetActive(true);

        while(currentTime < duration)
        {
            currentTime += Time.deltaTime;
            float alpha = currentTime / duration;

            canvas.BaseCanvasGroup.alpha = Mathf.Lerp(initialAlpha, targetAlpha, alpha);

            yield return null;
        }

        if (finishCallback != null)
            finishCallback.Invoke();
    }

    public bool ContainsCanvas(System.Type type)
    {
        List<BaseCanvas> mainCanvasList = new List<BaseCanvas>(m_UICanvasDictionary.Values.ToArray());
        return mainCanvasList.Exists(x => x.GetType().IsAssignableFrom(type));
    }

    public T GetCanvas<T>() where T :BaseCanvas
    {
        List<BaseCanvas> mainCanvasList = new List<BaseCanvas>(m_UICanvasDictionary.Values.ToArray());

        foreach(BaseCanvas canvas in mainCanvasList)
        {
            if (canvas.GetType().IsAssignableFrom(typeof(T)))
                return canvas as T;
        }

        return null;
    }

    public BaseCanvas GetCanvasByName(string Name)
    {
        return m_UICanvasDictionary[Name];
    }
}
