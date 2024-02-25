using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

[System.Serializable]
public class Speaker
{
    public string CharacterName;
}
[System.Serializable]
public struct DialogueLine
{
    public Speaker speaker;
    [TextArea(2, 5)]
    public string text;
}

[CreateAssetMenu(fileName = "New Dialogue", menuName = "Dialogue")]
public class Dialogue : ScriptableObject
{
    public Speaker speaker;
    public DialogueLine[] lines;
}
