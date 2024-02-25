using System.Collections;
using UnityEngine;
using UnityEngine.UI;
using TMPro;
using UnityEngine.Events;
using System.Linq;

public class DialogueManager : MonoBehaviour/*, ISaveableGameObject*/
{
    [Header("Dialogue Textbox objects to use for dialogues")]
    [SerializeField] private TextMeshProUGUI speakerTxt;
    [SerializeField] private TextMeshProUGUI text;
    [SerializeField] private Image speakerImage;
    [SerializeField] private ScriptableObjectDatabase dialogueDatabase;

    [Header("Diary Button")]
    [SerializeField] private GameObject diaryBtn;

    [Header("Delay time when typing each letter in a dialogue sentence"), Tooltip("Delay time when typing each letter in a dialogue sentence")]
    [SerializeField] private float typeSentenceDelay;


    private DialogueLine[] lines;
    private bool setInput;
    private bool dialogueOngoing = false;
    private bool closeButton = false;
    private bool isTyping;
    private bool sentenceCompleted;

    [Header("Dialogue Events")]
    [SerializeField] public UnityEvent EVT_OnFinishDialogueLine;
    [SerializeField] public UnityEvent EVT_OnStartDialogue;
    [SerializeField] public UnityEvent EVT_OnEndDialogue;

    public bool isDialogueOngoing { get { return dialogueOngoing; } }
    private int activeLineIndex = -1;
    private Dialogue currentDialogue;
    private DialogueHandler handler;
    private DialogueLineWithEvent diaLineEvent;

    public Dialogue currentDialog { get { return currentDialogue; } }

    Coroutine typeCoroutine = null;

    private void Awake()
    {
        SingletonManager.RegisterComponent(this);
        print("dialogue manager execute");
    }

    private void Start()
    {
        //handler = SingletonManager.GetSingleton<DialogueHandler>();

        EVT_OnStartDialogue.AddListener(HideDiaryButton);
        EVT_OnEndDialogue.AddListener(HideDiaryButton);
        isTyping = false;
    }

    public void ReplaceDialogueWithEvent(DialogueLineWithEvent DiaLineEventReplace)
    {
        diaLineEvent = DiaLineEventReplace;
    }

    public void AdvanceConvo()
    {
        Debug.Log("Sdsde Advance");
        sentenceCompleted = false;
        if (!isTyping)
            activeLineIndex++;

        if (isTyping)
        {
            Skip();
        }


        if (activeLineIndex < lines.Length && !isTyping && !sentenceCompleted)
        {
            speakerTxt.text = lines[activeLineIndex].speaker.CharacterName;
            text.text = lines[activeLineIndex].text;
            if (typeCoroutine != null)
                StopCoroutine(typeCoroutine);

            typeCoroutine = StartCoroutine(TypeSentence(lines[activeLineIndex].text));


        }

        else if(activeLineIndex >= lines.Length)
        {
            print("dialogue end");
            activeLineIndex = -1;
            OnEndDialogue();
            dialogueOngoing = false;
        }

    }

    private void Skip()
    {

        typeCoroutine = null;
        StopAllCoroutines();
        text.text = lines[activeLineIndex].text;
        text.maxVisibleCharacters = lines[activeLineIndex].text.Length;

        typeCoroutine = null;

        sentenceCompleted = true;
        isTyping = false;

    }

    public void StartDialogue(Dialogue dialogue, Speaker speaker, bool inputChange = true, float TimeScale = 0)
    {

        setInput = inputChange;

        OnStartDialogue(TimeScale);
        currentDialogue = dialogue;
        dialogueOngoing = true;
        speakerTxt.text = speaker.CharacterName;
        lines = dialogue.lines;

        AdvanceConvo();
    }
    
    IEnumerator TypeSentence(string sentence)
    {
   
        if (diaLineEvent != null && diaLineEvent.lineIndexWithEvtList.Any(x => x.indexWithEVT == activeLineIndex))
        {
            LineIndexWithEVT lineIndexEVT = diaLineEvent.lineIndexWithEvtList.Find(x => x.indexWithEVT == activeLineIndex);
            print("DialineEvent Start");
            lineIndexEVT.EVT_FireEventOnStartLine.Invoke();
        }

        isTyping = true;
        int lettersDisplayed;

        for(lettersDisplayed = 0; lettersDisplayed <= sentence.Length; lettersDisplayed++)
        {
            PlayerController pc = SingletonManager.GetSingleton<PersistentSceneManager>().Player.GetComponent<PlayerController>();

            if (pc.InputGameMode != EInputMode.Dialogue)
            {
                pc.SetInputGameMode(EInputMode.Dialogue);
            }
            text.maxVisibleCharacters = lettersDisplayed;
            yield return new WaitForSecondsRealtime(typeSentenceDelay);
        }

        if(lettersDisplayed >= sentence.Length)
        {
            print("sentence completed");
            EVT_OnFinishDialogueLine.Invoke();
            sentenceCompleted = true;

            if (diaLineEvent != null && diaLineEvent.lineIndexWithEvtList.Any(x => x.indexWithEVT == activeLineIndex))
            {
                LineIndexWithEVT lineIndexEVT = diaLineEvent.lineIndexWithEvtList.Find(x => x.indexWithEVT == activeLineIndex);
                print("DialineEvent End");
                lineIndexEVT.EVT_FireLineEventOnEndLine.Invoke();
            }
        }

        typeCoroutine = null;
        isTyping = false;
    }

    public void OnStartDialogue(float TimeScale = 0)
    {
        EVT_OnStartDialogue.Invoke();
        Time.timeScale = TimeScale;
    }

    public void OnEndDialogue(EInputMode defaultMode = EInputMode.Game)
    {
        Time.timeScale = 1;
        print("end dialogue");
        if (setInput)
        {
            SingletonManager.GetSingleton<PersistentSceneManager>().Player.GetComponent<PlayerController>().SetInputGameMode(defaultMode);
        }

        EVT_OnEndDialogue.Invoke();

    }

    public ESaveType GetSaveType()
    {
         return ESaveType.Scene;
    }

    private void HideDiaryButton()
    {
        if (!closeButton)
        {
            closeButton = true;
            //diaryBtn.SetActive(false);
        }
        else
        {
            closeButton = false;
            //diaryBtn.SetActive(true);
        }

    }

    public void TriggerDialogue(PlayerController player, Dialogue dialogueData)
    {
        print("trigger dialogue");
        player.SetInputGameMode(EInputMode.Dialogue);
        StartDialogue(dialogueData, dialogueData.speaker);

    }

}

